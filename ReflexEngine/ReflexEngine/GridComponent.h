#pragma once

#include "Component.h"
#include "VectorSet.h"
#include "Object.h"

namespace Reflex::Components
{
	typedef Handle< class Grid > GridHandle;
	typedef Handle< const class Grid > GridHandleConst;

	// Class definition
	class Grid : public Component
	{
	public:
		Grid( const unsigned width, const unsigned height, const float cellWidth, const float cellHeight );
		Grid( const sf::Vector2u gridSize, const sf::Vector2f cellSize );

		GridHandle GetHandle() { return GridHandle( shared_from_this() ); }
		GridHandleConst GetHandle() const { return GridHandleConst( shared_from_this() ); }

		void AddToGrid( const Reflex::ObjectHandle& handle, const unsigned x, const unsigned y );
		void AddToGrid( const Reflex::ObjectHandle& handle, const sf::Vector2u index );
		Reflex::ObjectHandle RemoveFromGrid( const unsigned x, const unsigned y );
		Reflex::ObjectHandle RemoveFromGrid( const sf::Vector2u index );
		Reflex::ObjectHandle GetCell( const unsigned x, const unsigned y ) const;
		Reflex::ObjectHandle GetCell( const sf::Vector2u index ) const;
		Reflex::ObjectHandle GetCell( const unsigned x, const unsigned y, const bool rotate ) const;
		Reflex::ObjectHandle GetCell( const sf::Vector2u index, const bool rotate ) const;

		unsigned GetWidth() const;
		unsigned GetHeight() const;
		sf::Vector2u GetGridSize() const;
		unsigned GetTotalCells() const;

		sf::Vector2f GetCellSize() const;
		void SetCellSize( const sf::Vector2f cellSize );

		void SetGridIsCentred( const bool centreGrid );
		bool GetGridIsCentred() const;

		sf::Vector2f GetCellPositionRelative( const sf::Vector2u index ) const;
		sf::Vector2f GetCellPositionWorld( const sf::Vector2u index ) const;
		std::pair< bool, sf::Vector2u > GetCellIndex( const sf::Vector2f worldPosition, bool rotated = true ) const;
		std::pair< bool, sf::Vector2u > ConvertCellIndex( const sf::Vector2u index, const bool rotate ) const;

		void ForEachChild( std::function< void( const Reflex::ObjectHandle& obj, const sf::Vector2u index ) > callback );

	protected:
		unsigned GetIndex( const sf::Vector2u coords ) const;
		const sf::Vector2u GetCoords( const unsigned index ) const;
		void UpdateGridPositions();

	private:
		Reflex::VectorSet< Reflex::ObjectHandle > m_children;
		sf::Vector2u m_gridSize;
		sf::Vector2f m_cellSize;
		bool m_centreGrid = true;
	};
}