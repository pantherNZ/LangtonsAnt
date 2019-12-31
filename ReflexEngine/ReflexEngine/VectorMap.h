#pragma once

namespace Reflex
{
	template< typename Key, typename Value >
	class VectorMap
	{
	public:
		typedef std::pair< Key, Value > value_type;
		typedef typename std::vector< value_type >::iterator iterator;
		typedef typename std::vector< value_type >::const_iterator const_iterator;

		VectorMap() {}
		explicit VectorMap( const unsigned initial_capacity ) { vec.reserve( initial_capacity ); }
		VectorMap( std::initializer_list< value_type > ilist ) : vec( ilist ) { std::sort( vec.begin(), vec.end(), KeysLess ); assert( std::adjacent_find( vec.cbegin(), vec.cend(), KeysEqual ) == vec.cend() ); }
		VectorMap( VectorMap< Key, Value >&& other ) noexcept : vec( std::move( other.vec ) ) {}

		iterator insert( const std::pair< Key, Value >& value );
		iterator insert( const std::pair< Key, Value >&& value );
		bool erase( const Key& key );
		iterator erase( iterator iter );
		iterator erase( const_iterator iter );

		iterator find( const Key& key );
		const_iterator find( const Key& key ) const;
		Value& operator[] ( const Key& key );

		void clear() { vec.clear(); }
		bool contains_key( const Key& key ) const;
		bool empty() const { return vec.empty(); }

		unsigned size() const { return vec.size(); }
		iterator begin() { return vec.begin(); }
		iterator end() { return vec.end(); }
		const_iterator begin() const { return vec.begin(); }
		const_iterator end() const { return vec.end(); }
		const_iterator cbegin() const { return vec.begin(); }
		const_iterator cend() const { return vec.end(); }

	private:
		static bool KeysEqual( const value_type& a, const value_type& b ) { return std::equal_to< Key >()( a.first, b.first ); }
		static bool KeysLess( const value_type& a, const value_type& b ) { return std::less< Key >()( a.first, b.first ); }

		template< typename Iterator, typename T >
		Iterator lower_bound_pair_first( const Iterator& begin, const Iterator& end, const T& t );

	private:
		std::vector< value_type > vec;
	};

	// Implementation
	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::iterator VectorMap< Key, Value >::insert( const std::pair< Key, Value >& value )
	{
		auto found = lower_bound_pair_first( begin(), end(), value.first );
		if( found != end() && found->first == value.first )
		{
			found->second = value.second;
			return found;
		}
		else
			return vec.insert( found, value );
	}

	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::iterator VectorMap< Key, Value >::insert( const std::pair< Key, Value >&& value )
	{
		auto found = lower_bound_pair_first( begin(), end(), value.first );
		if( found != end() && found->first == value.first )
		{
			found->second = value.second;
			return found;
		}
		else
			return vec.insert( found, std::move( value ) );
	}

	template< typename Key, typename Value >
	bool VectorMap< Key, Value >::erase( const Key& key )
	{
		auto found = lower_bound_pair_first( begin(), end(), key );
		if( found != end() && found->first == key )
		{
			vec.erase( found );
			return true;
		}
		return false;
	}

	template< typename Key, typename Value >
	template< typename Iterator, typename T >
	Iterator VectorMap< Key, Value >::lower_bound_pair_first( const Iterator& begin, const Iterator& end, const T& t )
	{
		typedef typename std::iterator_traits<Iterator>::value_type value_type;
		return std::lower_bound( begin, end, t, []( const value_type& a, const T& b )
		{
			return std::less< T >()( a.first, b );
		} );
	}

	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::iterator VectorMap< Key, Value >::erase( typename VectorMap< Key, Value >::iterator iter )
	{
		return vec.erase( iter );
	}

	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::iterator VectorMap< Key, Value >::erase( typename VectorMap< Key, Value >::const_iterator iter )
	{
		return vec.erase( iter );
	}

	template< typename Key, typename Value >
	bool VectorMap< Key, Value >::contains_key( const Key& key ) const
	{
		return find( key ) != end();
	}

	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::iterator VectorMap< Key, Value >::find( const Key& key )
	{
		auto found = lower_bound_pair_first( begin(), end(), key );
		return found != end() && found->first == key ? found : end();
	}

	template< typename Key, typename Value >
	typename VectorMap< Key, Value >::const_iterator VectorMap< Key, Value >::find( const Key& key ) const
	{
		auto found = lower_bound_pair_first( begin(), end(), key );
		return found != end() && found->first == key ? found : end();
	}

	template< typename Key, typename Value >
	Value& VectorMap< Key, Value >::operator[] ( const Key& key )
	{
		auto found = lower_bound_pair_first( begin(), end(), key );
		if( found == end() || found->first != key )
			found = vec.insert( found, std::make_pair( key, Value() ) );
		return found->second;
	}
}