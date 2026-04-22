#ifndef BOOK_PROJECTILE_HPP
#define BOOK_PROJECTILE_HPP

#include <Book/Entity.hpp>
#include <Book/ResourceIdentifiers.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>


class Projectile : public Entity
{
	public:
		enum Type
		{
			HelloKittyBullet,
			FruitBullet,
			Missile,
			Bomb,
			TypeCount
		};


	public:
								Projectile(Type type, const TextureHolder& textures);

		void					guideTowards(sf::Vector2f position);
		bool					isGuided() const;
		bool					isBomb() const;

		virtual unsigned int	getCategory() const;
		virtual sf::FloatRect	getBoundingRect() const;
		float					getMaxSpeed() const;
		int						getDamage() const;

	
	private:
		virtual void			updateCurrent(sf::Time dt, CommandQueue& commands);
		virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


	private:
		Type					mType;
		sf::Sprite				mSprite;
		sf::Time				mLifetime;
		bool					mHasTriggeredBombAttack;
		sf::Vector2f			mTargetDirection;
};

#endif // BOOK_PROJECTILE_HPP
