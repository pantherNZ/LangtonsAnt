#pragma once

#include "Component.h"
#include "Utility.h"
#include "InteractableSystem.h"
#include "SFMLObjectComponent.h"

namespace Reflex::Components
{
	typedef Handle< class Interactable > InteractableHandle;
	typedef Handle< const class Interactable > InteractableHandleConst;

	// Class definition
	class Interactable : public Component
	{
	public:
		friend class Reflex::Systems::InteractableSystem;

		Interactable( const SFMLObjectHandle& collisionObject = SFMLObjectHandle() );

		InteractableHandle GetHandle() { return InteractableHandle( shared_from_this() ); }
		InteractableHandleConst GetHandle() const { return InteractableHandleConst( shared_from_this() ); }

		// Settings, change as you want
		bool selectionIsToggle = true;
		bool unselectIfLostFocus = false;
		bool isEnabled = true;

		// Callbacks
		std::function< void( const Interactable&, const bool ) > focusChangedCallback;
		std::function< void( const Interactable&, const bool ) > selectionChangedCallback;

		bool IsFocussed() const;
		bool IsSelected() const;

	protected:
		void Select();
		void Deselect();

		bool isFocussed = false;
		bool isSelected = false;

		SFMLObjectHandle m_replaceCollisionObject;
	};
}