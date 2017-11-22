# -*- coding: utf-8 -*-
#
# test_basics.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

"""
Tests for basic topology hl_api functions.
"""

import unittest
import nest
import nest.topology as topo

try:
    import numpy

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


class BasicsTestCase(unittest.TestCase):
    def test_CreateLayer(self):
        """Creating a single layer from dict."""
        nr = 4
        nc = 5
        nest.ResetKernel()
        l = topo.CreateLayer({'elements': 'iaf_psc_alpha',
                              'rows': nr,
                              'columns': nc})
        self.assertEqual(len(l), nr * nc)

    def test_GetPosition(self):
        """Check if GetPosition returns proper positions."""
        pos = ((1.0, 0.0), (0.0, 1.0), (3.5, 1.5))
        ldict = {'elements': 'iaf_psc_alpha',
                 'extent': (20., 20.),
                 'positions': pos}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # GetPosition of single node
        nodepos_exp = topo.GetPosition(l[:1])
        self.assertEqual(nodepos_exp, pos[0])

        nodepos_exp = topo.GetPosition(l[-1:])
        self.assertEqual(nodepos_exp, pos[-1])

        nodepos_exp = topo.GetPosition(l[1:2])
        self.assertEqual(nodepos_exp, pos[1])

        # GetPosition of all the nodes in the layer
        nodepos_exp = topo.GetPosition(l)

        for npe, npr in zip(nodepos_exp, pos):
            self.assertEqual(npe, npr)

        self.assertEqual(pos, nodepos_exp)

        # GetPosition on some of the GIDs
        nodepos_exp = topo.GetPosition(l[:2])
        self.assertEqual(nodepos_exp, (pos[0], pos[1]))

    # TODO481 remove GetElement
    def test_GetElement(self):
        """Check if GetElement returns proper lists."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        checkpos = [[0, 0], [1, 1], [4, 3]]

        # single coord gives 1-elem gid list
        n1 = l.GetElement(checkpos[0])
        self.assertEqual(len(n1), 1)
        self.assertIsInstance(n1[0], int)
        self.assertEqual(n1[0], 1)

        # single gid, multiple coord gives len(checkpos) gid list
        n2 = l.GetElement(checkpos)
        self.assertEqual(len(n2), len(checkpos))
        self.assertTrue(nest.is_sequence_of_gids(n2))
        self.assertTrue(all(isinstance(n, int) for n in n2))
        self.assertEqual(n2, (1, 6, 20))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Displacement(self):
        """Interface check on displacement calculations."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        n = [x for x in l]

        # gids -> gids, all displacements must be zero here
        d = l.Displacement(n, n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(dd == (0., 0.) for dd in d))

        # single gid -> gids
        d = l.Displacement(n[:1], n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # gids -> single gid
        d = l.Displacement(n, n[:1])
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))
        
        # Displacement between gid 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the displacement on
        # x-axis should be approximately -dx, while the displacement on the
        # y-axis should be 0.
        d = l.Displacement(n[:1], n[4:5])
        dx = 1 / ldict['columns']
        self.assertAlmostEqual(d[0][0], -dx, 3)
        self.assertEqual(d[0][1], 0.0)
        
        # Displacement between gid 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the displacement on
        # x-axis should be 0, while the displacement on the y-axis should be
        # approximately dy.
        d = l.Displacement(n[:1], n[1:2])
        dy = 1 / ldict['rows']
        self.assertEqual(d[0][0], 0.0)
        self.assertAlmostEqual(d[0][1], dy, 3)

        from numpy import array

        # position -> gids
        d = l.Displacement(array([0.0, 0.0]), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # positions -> gids
        d = l.Displacement([array([0.0, 0.0])] * len(n), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Distance(self):
        """Interface check on distance calculations."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        n = [gid for gid in l]

        # gids -> gids, all displacements must be zero here
        d = l.Distance(n, n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([dd == 0. for dd in d]))

        # single gid -> gids
        d = l.Distance(n[:1], n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # gids -> single gid
        d = l.Distance(n, n[:1])
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))
        
        # Distance between gid 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the distance should be
        # approximately dx. The same is true for distance between gid 6 and 1.
        d = l.Distance(n[:1], n[4:5])
        dx = 1 / ldict['columns']
        self.assertAlmostEqual(d[0], dx, 3)
        
        d = l.Distance(n[4:5], n[:1])
        self.assertAlmostEqual(d[0], dx, 3)
        
        # Distance between gid 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the distance should be
        # approximately dy. The same is true for distance between gid 2 and 1.
        d = l.Distance(n[:1], n[1:2])
        dy = 1 / ldict['rows']
        self.assertAlmostEqual(d[0], dy, 3)
        
        d = l.Distance(n[1:2], n[:1])
        self.assertAlmostEqual(d[0], dy, 3)

        from numpy import array

        # position -> gids
        d = l.Distance(array([0.0, 0.0]), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # positions -> gids
        d = l.Distance([array([0.0, 0.0])] * len(n), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_FindElements(self):
        """Interface and result check for finding nearest element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': (3., 3.)}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # single location at center
        n = topo.FindNearestElement(l, (0., 0.))
        self.assertEqual(n, (5,))

        # two locations, one layer
        n = topo.FindNearestElement(l, ((0., 0.), (1., 1.)))
        self.assertEqual(n, (5, 7))

        # several closest locations, not all
        n = topo.FindNearestElement(l, (0.5, 0.5))
        self.assertEqual(len(n), 1)
        self.assertEqual(1, sum(n[0] == k for k in (4, 5, 7, 8)))

        # several closest locations, all
        n = topo.FindNearestElement(l, (0.5, 0.5), find_all=True)
        self.assertEqual(len(n), 1)
        self.assertEqual(n, ((4, 5, 7, 8),))

        # complex case
        n = topo.FindNearestElement(l, ((0., 0.), (0.5, 0.5)),
                                    find_all=True)
        self.assertEqual(n, ((5,), (4, 5, 7, 8)))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetCenterElement(self):
        """Interface and result check for finding center element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': (2., 2.)}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # single layer
        n = topo.FindCenterElement(l)
        self.assertEqual(n, 5)

        # new layer
        l2 = topo.CreateLayer(ldict)
        n = topo.FindCenterElement(l2)
        self.assertEqual(n, 14)

    def test_GetTargetNodesPositions(self):
        """Interface check for finding targets."""
        # Not allowed anymore
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3,
                 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        cdict = {'connection_type': 'divergent',
                 'synapse_model': 'stdp_synapse',
                 'mask': {'grid': {'rows': 2, 'columns': 2}}}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # connect l -> l
        topo.ConnectLayers(l, l, cdict)

        t = topo.GetTargetNodes(l[:1], l)
        self.assertEqual(len(t), 1)
        
        p = topo.GetTargetPositions(l[:1], l)
        self.assertEqual(len(p), 1)
        self.assertTrue(all([len(pp) == 2 for pp in p[0]]))

        t = topo.GetTargetNodes(l, l)
        self.assertEqual(len(t), len(l))
        # 2x2 mask -> four targets
        self.assertTrue(all([len(g) == 4 for g in t]))

        p = topo.GetTargetPositions(l, l)
        self.assertEqual(len(p), len(l))

        t = topo.GetTargetNodes(l, l, syn_model='static_synapse')
        self.assertEqual(len(t), len(l))
        self.assertTrue(all([len(g) == 0 for g in t]))  # no static syns

        t = topo.GetTargetNodes(l, l, syn_model='stdp_synapse')
        self.assertEqual(len(t), len(l))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets
        
        t = topo.GetTargetNodes(l[:1], l)
        self.assertEqual(t, ([1, 2, 4, 5],))
        
        t = topo.GetTargetNodes(l[4:5], l)
        self.assertEqual(t, ([5, 6, 8, 9],))
        
        t = topo.GetTargetNodes(l[8:9], l)
        self.assertEqual(t, ([1, 3, 7, 9],))

    def test_Parameter(self):

        x = topo.CreateParameter("constant", {"value": 2.0})
        y = topo.CreateParameter("constant", {"value": 3.0})

        z1 = x + y
        z2 = z1 * x
        z3 = z2 - x
        z4 = z3 / x
        z5 = z4 - y

        self.assertEqual(int(z1.GetValue((0.0, 0.0))), 5)
        self.assertEqual(int(z2.GetValue((1.0, 0.0))), 10)
        self.assertEqual(int(z3.GetValue((0.0, 1.0))), 8)
        self.assertEqual(int(z4.GetValue((1.0, 1.0))), 4)
        self.assertEqual(int(z5.GetValue((0.0, 0.0))), 1)


def suite():
    suite = unittest.makeSuite(BasicsTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
