#ifndef BOOK_PLAYER_HPP
#define BOOK_PLAYER_HPP

#include <Book/Command.hpp>
#include <Book/KeyBinding.hpp>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Window/Event.hpp>

#include <map>


class CommandQueue;

class Player : private sf::NonCopyable
{
	public:
		typedef PlayerAction::Type Action;

		enum MissionStatus
		{
			MissionRunning,
			MissionSuccess,
			MissionFailure
		};


	public:
								Player(sf::Int32 identifier, const KeyBinding* binding);

		void					handleEvent(const sf::Event& event, CommandQueue& commands);
		void					handleRealtimeInput(CommandQueue& commands);

		void 					setMissionStatus(MissionStatus status);
		MissionStatus 			getMissionStatus() const;


	private:
		void					initializeActions();


	private:
		const KeyBinding*			mKeyBinding;
		std::map<Action, Command>	mActionBinding;
		MissionStatus 				mCurrentMissionStatus;
		int							mIdentifier;
};

#endif // BOOK_PLAYER_HPP
