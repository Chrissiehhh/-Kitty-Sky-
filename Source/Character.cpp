#include <Book/Character.hpp>
#include <Book/DataTables.hpp>
#include <Book/Utility.hpp>
#include <Book/Pickup.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/SoundNode.hpp>
#include <Book/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>


using namespace std::placeholders;

namespace
{
	const std::vector<CharacterData> Table = initializeCharacterData();
	const sf::Vector2f ExplosionScale(0.7f, 0.7f);
	const sf::IntRect KittyRollFrames[] =
	{
		sf::IntRect(3, 4, 48, 52),
		sf::IntRect(56, 4, 38, 52),
		sf::IntRect(99, 4, 43, 52)
	};
}

Character::Character(Type type, const TextureHolder& textures, const FontHolder& fonts, State::Difficulty difficulty)
: Entity(Table[type].hitpoints)
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
, mExplosion(textures.get(Textures::Explosion))
, mFireCommand()
, mMissileCommand()
, mFireCountdown(sf::Time::Zero)
, mIsFiring(false)
, mIsLaunchingMissile(false)
, mShowExplosion(true)
, mExplosionBegan(false)
, mSpawnedPickup(false)
, mFireRateLevel(1)
, mSpreadLevel(1)
, mMissileAmmo(2)
, mDropPickupCommand()
, mTravelledDistance(0.f)
, mDirectionIndex(0)
, mMissileDisplay(nullptr)
, mProjectileSpeedFactor((mType == HelloKitty) ? 1.f : (difficulty == State::Simple ? 0.7f : 1.f))
, mIdentifier(0)
{
	mExplosion.setFrameSize(sf::Vector2i(256, 256));
	mExplosion.setNumFrames(16);
	mExplosion.setDuration(sf::seconds(1));
	mExplosion.setScale(ExplosionScale);

	centerOrigin(mSprite);
	centerOrigin(mExplosion);

	if (mType == HelloKitty)
		mSprite.setScale(2.f, 2.f);
	else
		mSprite.setScale(1.5f, 1.5f);

	mFireCommand.category = Category::SceneAirLayer;
	mFireCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createBullets(node, textures);
	};

	mMissileCommand.category = Category::SceneAirLayer;
	mMissileCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::Missile, 0.f, 0.5f, textures);
	};

	mDropPickupCommand.category = Category::SceneAirLayer;
	mDropPickupCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	attachChild(std::move(healthDisplay));

	if (getCategory() == Category::HelloKitty)
	{
		std::unique_ptr<TextNode> missileDisplay(new TextNode(fonts, ""));
		missileDisplay->setPosition(0, 92);
		mMissileDisplay = missileDisplay.get();
		attachChild(std::move(missileDisplay));
	}

	updateTexts();
}

int Character::getMissileAmmo() const
{
	return mMissileAmmo;
}

void Character::setMissileAmmo(int ammo)
{
	mMissileAmmo = ammo;
}

void Character::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isDestroyed() && mShowExplosion)
		target.draw(mExplosion, states);
	else
		target.draw(mSprite, states);
}

void Character::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	updateTexts();
	updateRollAnimation();

	// Entity has been destroyed: Possibly drop pickup, mark for removal
	if (isDestroyed())
	{
		checkPickupDrop(commands);
		mExplosion.update(dt);

		// Play explosion sound only once
		if (!mExplosionBegan)
		{
			// Play sound effect
			SoundEffect::ID soundEffect = (randomInt(2) == 0) ? SoundEffect::Explosion1 : SoundEffect::Explosion2;
			playLocalSound(commands, soundEffect);

			mExplosionBegan = true;
		}
		return;
	}

	// Check if bullets or missiles are fired
	checkProjectileLaunch(dt, commands);

	// Update enemy movement pattern; apply velocity
	updateMovementPattern(dt);
	Entity::updateCurrent(dt, commands);
}

unsigned int Character::getCategory() const
{
	if (isHelloKitty())
		return Category::HelloKitty;
	else
		return Category::Fruit;
}

sf::FloatRect Character::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool Character::isMarkedForRemoval() const
{
	return isDestroyed() && (mExplosion.isFinished() || !mShowExplosion);
}

void Character::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

bool Character::isHelloKitty() const
{
	return mType == HelloKitty;
}

float Character::getMaxSpeed() const
{
	return Table[mType].speed;
}

void Character::increaseFireRate()
{
	if (mFireRateLevel < 10)
		++mFireRateLevel;
}

void Character::increaseSpread()
{
	if (mSpreadLevel < 3)
		++mSpreadLevel;
}

void Character::collectMissiles(unsigned int count)
{
	mMissileAmmo += count;
}

void Character::fire()
{
	// Only ships with fire interval != 0 are able to fire
	if (Table[mType].fireInterval != sf::Time::Zero)
		mIsFiring = true;
}

void Character::launchMissile()
{
	if (mMissileAmmo > 0)
	{
		mIsLaunchingMissile = true;
		--mMissileAmmo;
	}
}

void Character::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition] (SoundNode& node, sf::Time)
		{
			node.playSound(effect, worldPosition); 
		});

	commands.push(command);
}

int	Character::getIdentifier()
{
	return mIdentifier;
}

void Character::setIdentifier(int identifier)
{
	mIdentifier = identifier;
}

void Character::updateMovementPattern(sf::Time dt)
{
	// Fruit movement pattern
	const std::vector<Direction>& directions = Table[mType].directions;
	if (!directions.empty())
	{
		// Moved long enough in current direction: Change direction
		if (mTravelledDistance > directions[mDirectionIndex].distance)
		{
			mDirectionIndex = (mDirectionIndex + 1) % directions.size();
			mTravelledDistance = 0.f;
		}

		// Compute velocity from direction
		float radians = toRadian(directions[mDirectionIndex].angle + 90.f);
		float vx = getMaxSpeed() * std::cos(radians);
		float vy = getMaxSpeed() * std::sin(radians);

		setVelocity(vx, vy);

		mTravelledDistance += getMaxSpeed() * dt.asSeconds();
	}
}

void Character::checkPickupDrop(CommandQueue& commands)
{
	// Fruit enemies drop at most one pickup, with a 50% chance
	if (!isHelloKitty() && randomInt(2) == 0 && !mSpawnedPickup)
		commands.push(mDropPickupCommand);

	mSpawnedPickup = true;
}

void Character::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	// Fruits try to fire all the time
	if (!isHelloKitty())
		fire();

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero)
	{
		// Interval expired: We can fire a new bullet
		commands.push(mFireCommand);
		playLocalSound(commands, isHelloKitty() ? SoundEffect::HelloKittyGunfire : SoundEffect::FruitGunfire);

		mFireCountdown += Table[mType].fireInterval / (mFireRateLevel + 1.f);
		mIsFiring = false;
	}
	else if (mFireCountdown > sf::Time::Zero)
	{
		// Interval not expired: Decrease it further
		mFireCountdown -= dt;
		mIsFiring = false;
	}

	// Check for missile launch
	if (mIsLaunchingMissile)
	{
		commands.push(mMissileCommand);
		playLocalSound(commands, SoundEffect::LaunchMissile);

		mIsLaunchingMissile = false;
	}
}

void Character::createBullets(SceneNode& node, const TextureHolder& textures) const
{
	Projectile::Type type = isHelloKitty() ? Projectile::HelloKittyBullet : Projectile::FruitBullet;

	switch (mSpreadLevel)
	{
		case 1:
			createProjectile(node, type, 0.0f, 0.5f, textures);
			break;

		case 2:
			createProjectile(node, type, -0.33f, 0.33f, textures);
			createProjectile(node, type, +0.33f, 0.33f, textures);
			break;

		case 3:
			createProjectile(node, type, -0.5f, 0.33f, textures);
			createProjectile(node, type,  0.0f, 0.5f, textures);
			createProjectile(node, type, +0.5f, 0.33f, textures);
			break;
	}
}

void Character::createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);
	sf::Vector2f velocity(0, projectile->getMaxSpeed());

	float sign = isHelloKitty() ? -1.f : +1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	projectile->setVelocity(velocity * sign * mProjectileSpeedFactor);
	node.attachChild(std::move(projectile));
}

void Character::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::Type>(randomInt(Pickup::TypeCount));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}

void Character::updateTexts()
{
	// Display hitpoints
	if (isDestroyed())
		mHealthDisplay->setString("");
	else
		mHealthDisplay->setString(toString(getHitpoints()) + " HP");
	mHealthDisplay->setPosition(0.f, isHelloKitty() ? 65.f : 50.f);
	mHealthDisplay->setRotation(-getRotation());

	// Display missiles, if available
	if (mMissileDisplay)
	{
		if (mMissileAmmo == 0 || isDestroyed())
			mMissileDisplay->setString("");
		else
			mMissileDisplay->setString("M: " + toString(mMissileAmmo));
	}
}

void Character::updateRollAnimation()
{
	if (Table[mType].hasRollAnimation)
	{
		sf::IntRect textureRect = KittyRollFrames[0];

		// Roll left: Texture rect offset once
		if (getVelocity().x < 0.f)
			textureRect = KittyRollFrames[1];

		// Roll right: Texture rect offset twice
		else if (getVelocity().x > 0.f)
			textureRect = KittyRollFrames[2];

		mSprite.setTextureRect(textureRect);
		centerOrigin(mSprite);
	}
}
