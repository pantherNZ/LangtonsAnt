#pragma once

namespace Reflex::Core { class Object; }

// Common Utility
namespace Reflex
{
	template< class T >
	class Handle
	{
	public:
		Handle() {}
		Handle( const std::shared_ptr< T >& item ) { ptr = std::weak_ptr( item ); }
		T* Get() const { return ptr.lock().get(); }
		std::shared_ptr< T > Lock() { return ptr.lock(); }
		std::shared_ptr< T > Lock() const { return ptr.lock(); }
		T* operator->() { return Get(); }
		T* operator->() const { return Get(); }
		bool IsValid() const { return !ptr.expired(); }
		void Reset() { ptr.reset(); }

		bool operator==( const Handle< const T >& other ) const { return Get() == other.Get(); }
		T& operator*() const { return *Get(); }
		operator bool() const { return IsValid(); }

		template< class V >
		Handle( const Handle< V >& handle )
			: ptr( std::weak_ptr( std::static_pointer_cast< T >( handle.Lock() ) ) ) { }

		template< class V >
		Handle( const std::shared_ptr< V >& other )
			: ptr( std::weak_ptr( std::static_pointer_cast< T >( other ) ) ) { }

	protected:
		std::weak_ptr< T > ptr;
	};

	//namespace std
	//{
	//	template< class T >
	//	struct hash< Handle< T > >
	//	{
	//		std::size_t operator()( const Handle< T >& k ) const
	//		{
	//			return hash< uint32_t >()( k.m_index ) ^ ( hash< uint32_t >()( k.m_counter ) >> 1 );
	//		}
	//	};
	//}

	typedef Handle< class Reflex::Core::Object > ObjectHandle;
}