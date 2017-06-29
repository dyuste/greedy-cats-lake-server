/*
 *  Copyright (c) 2015 David Yuste Romero
 *
 *  THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 *  OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 *  Permission is hereby granted to use or copy this program
 *  for any purpose,  provided the above notices are retained on all copies.
 *  Permission to modify the code and to distribute modified code is granted,
 *  provided the above notices are retained, and a notice that the code was
 *  modified is included with the above copyright notice.
 */
 #ifndef MISC_UTILITIES_MINROUTE_H_
#define MISC_UTILITIES_MINROUTE_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <ctime>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/timer.hpp>
#include <boost/integer_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/graph/metric_tsp_approx.hpp>
#include <boost/graph/graphviz.hpp>

namespace misc {
	template <typename coordinate_type, typename annotation_type>
	struct annotated_point : public boost::simple_point<coordinate_type> {
		annotation_type annotation;
	};

	template <typename coordinate_type, typename annotation_type>
	class annotated_position_vector : public std::vector<annotated_point<coordinate_type, annotation_type> > {
	public:
		inline void addVertex(coordinate_type x, coordinate_type y, const annotation_type & ann) {
			annotated_point<coordinate_type, annotation_type> point;
			point.x = x;
			point.y = y;
			point.annotation = ann;
			this->push_back(point);
		}
	};
		
	class MinRoute {
	public:
		template <typename coordinate_type, typename annotation_type>
		static bool Compute(annotated_position_vector<coordinate_type, annotation_type> & position_vec, std::vector<annotation_type> & path, coordinate_type & len)
		{
			using namespace boost;
			using namespace std;

			typedef adjacency_matrix<undirectedS, no_property,
				property <edge_weight_t, double> > Graph;
			typedef graph_traits<Graph>::vertex_descriptor Vertex;
			typedef property_map<Graph, edge_weight_t>::type WeightMap;
			typedef property_map<Graph, vertex_index_t>::type VertexMap;

			Graph g(position_vec.size());
			WeightMap weight_map(get(edge_weight, g));
			VertexMap vertex_index_map = get(vertex_index, g);

			connectAllEuclidean(g, position_vec, weight_map, vertex_index_map, position_vec.size());
			
			vector<Vertex> c;
			len = static_cast<coordinate_type>(0.0);
			try {
				metric_tsp_approx(g, make_tsp_tour_len_visitor(g, back_inserter(c), len, weight_map));
			}
			catch (const bad_graph& e) {
				return false;
			}

			for (vector<Vertex>::iterator itr = c.begin(); itr != c.end(); ++itr)
				path.push_back(position_vec[vertex_index_map[*itr]].annotation);

			return true;
		}
		
	private:
		template<typename VertexListGraph, typename PointContainer,
			typename WeightMap, typename VertexIndexMap>
		static inline void connectAllEuclidean(VertexListGraph& g,
							const PointContainer& points,
							WeightMap wmap,				// Property maps passed by value
							VertexIndexMap vmap,		// Property maps passed by value
							int /*sz*/)
		{
			using namespace boost;
			using namespace std;
			typedef typename graph_traits<VertexListGraph>::vertex_iterator VItr;

			typename graph_traits<VertexListGraph>::edge_descriptor e;
			bool inserted;

			pair<VItr, VItr> verts(vertices(g));
			for (VItr src(verts.first); src != verts.second; src++)
			{
				for (VItr dest(src); dest != verts.second; dest++)
				{
						if (dest != src)
						{
							double weight(sqrt(pow(
								static_cast<double>(points[vmap[*src]].x -
										points[vmap[*dest]].x), 2.0) +
								pow(static_cast<double>(points[vmap[*dest]].y -
										points[vmap[*src]].y), 2.0)));

							boost::tie(e, inserted) = add_edge(*src, *dest, g);

							wmap[e] = weight;
						}

				}

			}
		}
	};
}

#endif