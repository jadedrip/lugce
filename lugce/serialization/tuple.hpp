#pragma once
#include <boost/tuple/tuple.hpp>

namespace boost{
	namespace serialization{
		template<class Archive, class T1, class T2>
		inline void serialize(Archive & ar, boost::tuple<T1,T2>& g, const unsigned int)
		{
			ar & boost::get<0>(g);
			ar & boost::get<1>(g);
		}
		template<class Archive, class T1, class T2, class T3>
		inline void serialize(Archive & ar, boost::tuple<T1,T2,T3>& g, const unsigned int)
		{
			ar & boost::get<0>(g);
			ar & boost::get<1>(g);
			ar & boost::get<2>(g);
		}
		template<class Archive, class T1, class T2, class T3, class T4>
		inline void serialize(Archive & ar, boost::tuple<T1,T2,T3,T4>& g, const unsigned int)
		{
			ar & boost::get<0>(g);
			ar & boost::get<1>(g);
			ar & boost::get<2>(g);
			ar & boost::get<3>(g);
		}
		template<class Archive, class T1, class T2, class T3, class T4, class T5>
		inline void serialize(Archive & ar, boost::tuple<T1,T2,T3,T4,T5>& g, const unsigned int)
		{
			ar & boost::get<0>(g);
			ar & boost::get<1>(g);
			ar & boost::get<2>(g);
			ar & boost::get<3>(g);
			ar & boost::get<4>(g);
		}
		template<class Archive, class T1, class T2, class T3, class T4, class T5, class T6>
		inline void serialize(Archive & ar, boost::tuple<T1,T2,T3,T4,T5,T6>& g, const unsigned int)
		{
			ar & boost::get<0>(g);
			ar & boost::get<1>(g);
			ar & boost::get<2>(g);
			ar & boost::get<3>(g);
			ar & boost::get<4>(g);
			ar & boost::get<5>(g);
		}
	};
};
