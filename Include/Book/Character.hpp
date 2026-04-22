#ifndef BOOK_CHARACTER_HPP
#define BOOK_CHARACTER_HPP

#include <Book/Entity.hpp>
#include <Book/Command.hpp>
#include <Book/ResourceIdentifiers.hpp>
#include <Book/Projectile.hpp>
#include <Book/TextNode.hpp>
#include <Book/Animation.hpp>
#include <Book/State.hpp>

#include <SFML/Graphics/Sprite.hpp>


class Character : public Entity
{
	public:
		enum Type
		{
			HelloKitty,
			Apple,
			Banana,
			PineappleBoss,
			TypeCount
		};


	public:
								Character(Type type, const TextureHolder& textures, const FontHolder& fonts, State::Difficulty difficulty);

		virtual unsigned int	getCategory() const;
		virtual sf::FloatRect	getBoundingRect() const;
		virtual void			remove();
		virtual bool 			isMarkedForRemoval() const;
		bool					isHelloKitty() const;
		bool					isBoss() const;
		float					getMaxSpeed() const;

		void					increaseFireRate();
		void					increaseSpread();
		void					collectMissiles(unsigned int count);
		void					collectBombs(unsigned int count);

		void 					fire();
		void					launchMissile();
		void					launchBomb();
		void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
		int						getIdentifier();
		void					setIdentifier(int identifier);
		int						getMissileAmmo() const;
		void					setMissileAmmo(int ammo);
		int						getBombAmmo() const;


	private:
		virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
		virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
		void					updateMovementPattern(sf::Time dt);
		void					checkPickupDrop(CommandQueue& commands);
		void					checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

		void					createBullets(SceneNode& node, const TextureHolder& textures) const;
		void					createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const;
		void					createPickup(SceneNode& node, const TextureHolder& textures) const;

		void					updateTexts();
		void					updateRollAnimation();


	private:
		Type					mType;
		sf::Sprite				mSprite;
		Animation				mExplosion;
		Command 				mFireCommand;
		Command					mMissileCommand;
		Command					mBombCommand;
		sf::Time				mFireCountdown;
		bool 					mIsFiring;
		bool					mIsLaunchingMissile;
		bool					mIsLaunchingBomb;
		bool 					mShowExplosion;
		bool					mExplosionBegan;
		bool					mSpawnedPickup;

		int						mFireRateLevel;
		int						mSpreadLevel;
		int						mMissileAmmo;
		int						mBombAmmo;

		Command 				mDropPickupCommand;
		float					mTravelledDistance;
		std::size_t				mDirectionIndex;
		TextNode*				mHealthDisplay;
		TextNode*				mMissileDisplay;
		TextNode*				mBombDisplay;
		State::Difficulty		mDifficultyLevel;
		float					mProjectileSpeedFactor;
	
		int						mIdentifier;
};

#endif // BOOK_CHARACTER_HPP
