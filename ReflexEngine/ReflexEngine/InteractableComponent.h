#pragma once

#include "Component.h"
#include "Utility.h"
#include "InteractableSystem.h"
#include "SFMLObjectComponent.h"

namespace Reflex::Components
{
	// Class definition
	DefineComponent( Interactable )
		friend class Reflex::Systems::InteractableSystem;

		Interactable( const SFMLObjectHandle& collisionObject = SFMLObjectHandle() );

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