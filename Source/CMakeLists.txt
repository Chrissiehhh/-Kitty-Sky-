
set(SRC
	Animation.cpp
	Application.cpp
	Button.cpp
	BloomEffect.cpp
	Command.cpp
	CommandQueue.cpp
	Component.cpp
	Container.cpp
	DataTables.cpp
	EmitterNode.cpp
	Entity.cpp
	GameOverState.cpp
	GameState.cpp
	KeyBinding.cpp
	Label.cpp
	MenuState.cpp
	MusicPlayer.cpp
	PauseState.cpp
	ParticleNode.cpp
	Pickup.cpp
	Character.cpp
	Player.cpp
	PostEffect.cpp
	Projectile.cpp
	SceneNode.cpp
	SettingsState.cpp
	SpriteNode.cpp
	TextNode.cpp
	SoundNode.cpp
	SoundPlayer.cpp
	State.cpp
	StateStack.cpp
	TitleState.cpp
	Utility.cpp
	World.cpp
)

add_executable(SFMLDemo Main.cpp ${SRC})

target_include_directories(SFMLDemo PRIVATE "${CMAKE_SOURCE_DIR}/Include")

if(SFML_STATIC_LIBRARIES)
	target_compile_definitions(SFMLDemo PRIVATE SFML_STATIC)
endif()

# SFML 2.x official config uses lowercase hyphenated targets (see SFMLConfig.cmake).
target_link_libraries(SFMLDemo PRIVATE
	sfml-graphics
	sfml-window
	sfml-system
	sfml-audio
)
