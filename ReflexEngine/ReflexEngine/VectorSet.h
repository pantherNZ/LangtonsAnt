#pragma once

namespace Reflex
{
	template< typename T >
	class VectorSet
	{
	public:
		typedef T value_type;
		typedef typename std::vector< T >::iterator iterator;
		typedef typename std::vector< T >::const_iterator const_iterator;

		VectorSet() { }
		explicit VectorSet( const unsigned initial_capacity ) { m_vector.reserve( initial_capacity ); }
		explicit VectorSet( std::vector< T >&& v ) noexcept : m_vector( std::move( v ) ) { validate(); }
		VectorSet( iterator begin, iterator end ) : m_vector( begin, end ) { validate(); }
		VectorSet( const_iterator begin, const_iterator end ) : m_vector( begin, end ) { validate(); }
		VectorSet( VectorSet< T >&& other ) noexcept : m_vector( std::move( other.m_vector ) ) {}
		VectorSet( const VectorSet< T >& other ) : m_vector( other.m_vector ) {}

		VectorSet( std::initializer_list< T > ilist ) : m_vector( std::move( ilist ) ) { validate(); }

		bool insert( const T& t );
		bool insert( T&& t );
		bool erase( const T& t );
		bool erase( const iterator& t );

		template< typename Pred >
		void erase_if( Pred pred );

		iterator find( const T& t );
		const_iterator find( const T& t ) const;

		void clear() { m_vector.clear(); }
		bool contains( const T& t ) const { return Find( t ) != end(); }
		bool empty() const { return m_vector.empty(); }
		void resize( const unsigned size ) { m_vector.resize( size ); }
		unsigned size() const { return ( unsigned )m_vector.size(); }

		iterator begin() { return m_vector.begin(); }
		iterator end() { return m_vector.end(); }
		const_iterator begin() const { return m_vector.begin(); }
		const_iterator end() const { return m_vector.end(); }
		const_iterator cbegin() const { return begin(); }
		const_iterator cend() const { return end(); }

		const std::vector< T >& get_vector() const { return m_vector; }
		std::vector< T >& get_vector() { return m_vector; }
		std::vector< T > release_vector() { return std::move( m_vector ); }

		VectorSet< T >& operator =( const VectorSet< T >& other ) { m_vector = other.m_vector; return *this; }
		const T& operator []( unsigned index ) const { return m_vector[index]; }
		T& operator []( unsigned index ) { return m_vector[index]; }

	private:
		std::vector< T > m_vector;

		void validate() { std::sort( m_vector.begin(), m_vector.end() ); m_vector.erase( std::unique( m_vector.begin(), m_vector.end() ), m_vector.end() ); }
	};

	template< typename T >
	bool VectorSet< T >::insert( const T& t )
	{
		auto found = std::lower_bound( begin(), end(), t );
		if( found == end() || *found != t )
		{
			m_vector.insert( found, t );
			return true;
		}
		return false;
	}

	template< typename T >
	bool VectorSet< T >::insert( T&& t )
	{
		auto found = std::lower_bound( begin(), end(), t );
		if( found == end() || *found != t )
		{
			m_vector.insert( found, std::move( t ) );
			return true;
		}
		return false;
	}

	template< typename T >
	bool VectorSet< T >::erase( const T& t )
	{
		auto found = std::lower_bound( begin(), end(), t );
		if( found != end() && *found == t )
		{
			m_vector.erase( found );
			return true;
		}
		return false;
	}

	template< typename T >
	bool VectorSet< T >::erase( typename const VectorSet< T >::iterator& t )
	{
		if( t == end() )
			return false;

		m_vector.erase( t );
		return true;
	}

	template< typename T >
	template< typename Pred >
	void VectorSet< T >::erase_if( Pred pred )
	{
		m_vector.erase( std::remove_if( begin(), end(), pred ), end() );
	}

	template< typename T >
	typename VectorSet< T >::iterator VectorSet< T >::find( const T& t )
	{
		auto found = std::lower_bound( begin(), end(), t );
		return found != end() && *found == t ? std::move( found ) : end();
	}

	template< typename T >
	typename VectorSet< T >::const_iterator VectorSet< T >::find( const T& t ) const
	{
		auto found = std::lower_bound( begin(), end(), t );
		return found != end() && *found == t ? std::move( found ) : end();
	}
}