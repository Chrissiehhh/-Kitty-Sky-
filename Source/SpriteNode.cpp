#include <Book/SpriteNode.hpp>

#include <SFML/Graphics/RenderTarget.hpp>


SpriteNode::SpriteNode(const sf::Texture& texture)
: mSprite(texture)
, mFlipVertically(false)
{
}	

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect, bool flipVertically)
: mSprite(texture, textureRect)
, mFlipVertically(flipVertically)
{
}

void SpriteNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (mFlipVertically)
	{
		sf::Sprite sprite(mSprite);
		sf::FloatRect bounds = sprite.getLocalBounds();
		sprite.setOrigin(bounds.left, bounds.top + bounds.height);
		sprite.setScale(1.f, -1.f);
		target.draw(sprite, states);
	}
	else
	{
		target.draw(mSprite, states);
	}
}
