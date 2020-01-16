#pragma once

#include "Precompiled.h"

#undef GetObject

namespace Reflex::Core { class Object; }

namespace Reflex::Components
{
	typedef Handle< class Component > ComponentHandle;
	typedef Handle< const class Component > ComponentHandleConst;

	class Component : public std::enable_shared_from_this< Component >
	{
	public:
		friend class Reflex::Core::Object;

		ObjectHandle GetObject() const { return m_object; }
		ComponentHandle GetHandle() { return ComponentHandle( shared_from_this() ); }
		ComponentHandleConst GetHandle() const { return ComponentHandleConst( shared_from_this() ); }
		virtual void SetOwningObject( const ObjectHandle& owner ) { m_object = owner; }
		virtual void OnConstructionComplete() {}

	protected:
		virtual void OnRemoved() {}

		Component() { }
		virtual ~Component() { }

	protected:
		ObjectHandle m_object;
	};

#define DefineComponent( X ) \
	typedef Handle< class X > X##Handle; \
	typedef Handle< const class X > X##HandleConst; \
	\
	class X : public Component \
	{ \
	public: \
		X##Handle GetHandle() { return X##Handle( shared_from_this() ); } \
		X##HandleConst GetHandle() const { return X##HandleConst( shared_from_this() ); }

#define DefineComponentWithInheritence( X, Y ) \
	typedef Handle< class X > X##Handle; \
	typedef Handle< const class X > X##HandleConst; \
	\
	class X : public Component, public Y \
	{ \
	public: \
		X##Handle GetHandle() { return X##Handle( shared_from_this() ); } \
		X##HandleConst GetHandle() const { return X##HandleConst( shared_from_this() ); } 
}