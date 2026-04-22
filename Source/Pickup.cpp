#include <Book/Pickup.hpp>
#include <Book/DataTables.hpp>
#include <Book/Category.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/Utility.hpp>
#include <Book/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>


namespace
{
	const std::vector<PickupData> Table = initializePickupData();
}

Pickup::Pickup(Type type, const TextureHolder& textures)
: Entity(1)
, mType(type)
, mSprite()
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

	if (mType == BombCrate)
		mSprite.setScale(0.35f, 0.35f);
}

unsigned int Pickup::getCategory() const
{
	return Category::Pickup;
}

sf::FloatRect Pickup::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Pickup::apply(Character& player) const
{
	Table[mType].action(player);
}

void Pickup::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}
