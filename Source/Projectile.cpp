#include <Book/Projectile.hpp>
#include <Book/Character.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/EmitterNode.hpp>
#include <Book/DataTables.hpp>
#include <Book/Utility.hpp>
#include <Book/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>
#include <cassert>


namespace
{
	const std::vector<ProjectileData> Table = initializeProjectileData();
}

Projectile::Projectile(Type type, const TextureHolder& textures)
: Entity(1)
, mType(type)
, mSprite()
, mLifetime(sf::Time::Zero)
, mHasTriggeredBombAttack(false)
, mTargetDirection()
{
	sf::IntRect textureRect = Table[type].textureRect;
	if (textureRect.width == 0 || textureRect.height == 0)
	{
		sf::Vector2u textureSize = textures.get(Table[type].texture).getSize();
		textureRect = sf::IntRect(0, 0, static_cast<int>(textureSize.x), static_cast<int>(textureSize.y));
	}

	mSprite.setTexture(textures.get(Table[type].texture));
	mSprite.setTextureRect(textureRect);
	centerOrigin(mSprite);

	// Add particle system for missiles
	if (isGuided())
	{
		std::unique_ptr<EmitterNode> smoke(new EmitterNode(Particle::Smoke));
		smoke->setPosition(0.f, getBoundingRect().height / 2.f);
		attachChild(std::move(smoke));

		std::unique_ptr<EmitterNode> propellant(new EmitterNode(Particle::Propellant));
		propellant->setPosition(0.f, getBoundingRect().height / 2.f);
		attachChild(std::move(propellant));

	}
	else if (isBomb())
	{
		mSprite.setScale(0.28f, 0.28f);
	}
}

void Projectile::guideTowards(sf::Vector2f position)
{
	assert(isGuided());
	mTargetDirection = unitVector(position - getWorldPosition());
}

bool Projectile::isGuided() const
{
	return mType == Missile;
}

bool Projectile::isBomb() const
{
	return mType == Bomb;
}

void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	mLifetime += dt;

	if (isGuided())
	{
		const float approachRate = 200.f;

		sf::Vector2f newVelocity = unitVector(approachRate * dt.asSeconds() * mTargetDirection + getVelocity());
		newVelocity *= getMaxSpeed();
		float angle = std::atan2(newVelocity.y, newVelocity.x);

		setRotation(toDegree(angle) + 90.f);
		setVelocity(newVelocity);
	}
	else if (isBomb())
	{
		rotate(260.f * dt.asSeconds());

		if (!mHasTriggeredBombAttack)
		{
			Command bombSweep;
			bombSweep.category = Category::Fruit;
			bombSweep.action = derivedAction<Character>([this] (Character& enemy, sf::Time)
			{
				if (enemy.isDestroyed())
					return;

				const float blastRadius = 1050.f;
				if (length(enemy.getWorldPosition() - getWorldPosition()) > blastRadius)
					return;

				if (enemy.isBoss())
				{
					int remainingAfterBlast = std::max(1, enemy.getHitpoints() / 2);
					enemy.setHitpoints(remainingAfterBlast);
				}
				else
				{
					enemy.destroy();
				}
			});

			commands.push(bombSweep);
			mHasTriggeredBombAttack = true;
		}
	}

	Entity::updateCurrent(dt, commands);
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

unsigned int Projectile::getCategory() const
{
	if (mType == FruitBullet)
		return Category::FruitProjectile;
	else
		return Category::HelloKittyProjectile;
}

sf::FloatRect Projectile::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

float Projectile::getMaxSpeed() const
{
	return Table[mType].speed;
}

int Projectile::getDamage() const
{
	return Table[mType].damage;
}
