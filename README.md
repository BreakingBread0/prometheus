# Prometheus <!-- omit in toc -->

Hey and welcome, thanks for stopping by!
- [0. Thank you](#0-thank-you)
- [1. Usage](#1-usage)
  - [Download a Release](#download-a-release)
    - [Optional stuff](#optional-stuff)
    - [Patcher \& how it works](#patcher--how-it-works)
  - [Compilation](#compilation)
- [2. Game internals](#2-game-internals)
  - [Managers](#managers)
  - [ECS](#ecs)
  - [STU](#stu)
  - [DataFlow](#dataflow)
  - [Game Messages](#game-messages)
  - [Components](#components)
    - [STUPvPGameComponent (0x24)](#stupvpgamecomponent-0x24)
    - [STUStatescriptComponent (0x23)](#stustatescriptcomponent-0x23)
    - [SceneRendering Component (1)](#scenerendering-component-1)
    - [MovementStateSystem (0x12, 0x15, 0x16)](#movementstatesystem-0x12-0x15-0x16)
    - [Embedded / LobbyMap Entity Admin (0x54)](#embedded--lobbymap-entity-admin-0x54)
- [3. Broken stuff and why it's broken](#3-broken-stuff-and-why-its-broken)
- [4. Tips \& Tricks](#4-tips--tricks)
- [5. Contributions Welcome!](#5-contributions-welcome)
- [6. NOTE](#6-note)
- [7. Open Source libraries used](#7-open-source-libraries-used)
- [8. License and Contact](#8-license-and-contact)

# 0. Thank you

As I've said in [my talk](https://media.ccc.de/v/39c3-prometheus-reverse-engineering-overwatch), this was and is a labor of love. Please do not send any hate to current or past Overwatch developers, and please do not harrass them. It is still my favourite multiplayer game, and if you're reading this, chances are it is yours too. It probably wasn't easy to just stomp out a whole new multiplayer game within two years out of the broken shambles of Titan. 0.8, even if still a "beta" version, is very much feature-complete and very stable!

# 1. Usage

This project is more an exploration of all the structures and the general game layout. It is not ready, and you cannot play with other people. A lot of hero abilities don't work, including shooting weapons.

If you havent already, download the 0.8 beta from [archive.org](https://archive.org/details/overwatch-beta-0-8-0-24919) and extract the files somewhere.
* ⚠️ Make sure that you don't download any malicious executable and verify that GameClientApp.exe is signed by Blizzard. The signature will get broken after applying the patcher.
* You can safely remove the BlizzardError directory.

## Download a Release

* Download a release.zip. Extract all files to the game directory.
* Rename prometheus.dll into inject.dll
* Execute patcher.exe. It will ask for an input GameClientApp.exe, select the one you have downloaded and verified.
  * The patcher will write a GameClientApp.patched.exe file into the same directory. The only thing this patched executable does is load inject.dll before running the game code itself.
* Congratulations, you're done :) Have a cookie 🍪

### Optional stuff

* Download the [MonaspaceXenon](https://monaspace.githubnext.com/) font and put the -regular.otf and -bold.otf in the directory of the game files.
* Download the [Font Awesome v6](https://fontawesome.com/v6/download) free desktop font files and put the .otf files into the game directory.
* Once first started, the library will create hashlibrary.json. You can add crc32 strings / elements to hash which will be displayed in various places where applicable. You can just add all the strings from the [overtools github repository](https://github.com/overtools/OWLib/tree/develop/TankLibHelper/DataPreHashChange). To do so add another root JSON element (an array) called "add" and put all your strings there. See the [json schema](hashlibrary.schema.json).

### Patcher & how it works

The patcher itself just goes to TlsCallback_0 and patches some bytes so the executable loads inject.dll before anything else. inject.dll then restores the game to its original state, decrypts everything and hooks stuff. Afterwards it runs the game normally. This was achieved with just copying the bytes which lazy_importer created for the LoadLibrary function and then calling LoadLibrary with "inject.dll" as an argument.

## Compilation

* You need Visual Studio 2022. I have not tested it on any other platform
* Make sure to initialize the submodules / clone recursively
* Initialize the vcpkg repository with the ps1 file located at external/vcpkg/scripts/bootstrap.ps1
* Compile as Relaese/x64. No other configuration is tested  (some flags and settings are missing).

# 2. Game internals

## Managers
The first think Overwatch does is initialize all its "Managers". They handle things such as Dataflow, CASC, Window management, etc. This is the lowest level and not really interesting.

## ECS
Afterwards, all the Entity Admins get initialized. First the Lobby, then Game and finally Replay.

There is one Entity admin base class on which all others depend upon. The LobbyEntityAdmin includes some login and user information in the inherited class, though I have never really researched that.

The Game Enttiy Admin is the biggest one and contains amongst others:
* An uint that says "this is the local entity". This must reference the controller entity. Systems use Components 2F (local player component) and 20 (model reference component) out of that entity to display the local player on screen.
* A JAM virtual function table. This is used to send JAM messages contained in actions.
* A table of map callbacks. Systems can subscribe to map change events (for example if the map state changes from "Global Loading" to "World Loading")
* A virtual function table reference from the MirrorSystem, which systems can use to subscribe to pre and post component deserialization. When a server sends a component update it calls the callback functions that were registered.
* An Accessor which searches for an Entity with the PvPGameComponent and takes some information from that.
* An entityid referencing a static dataflow provider?
* A char indicating if this is the live game or not.

The replay entity admin is just a GameEntityAdmin with a few additional systems and the JAM sending vtable just being return 0 functions (lmao).

## STU

STU is the proprietary data format for Overwatch. It gets used to read data out of CASC archives and some ingame data gets sent from the server using this. All things needed to read out of STU are located in STU.h. If you want to edit STU, include STU_Editable.h but make sure to read the comments so you dont crash ;).

For usage examples you can have a look in windows/stu_explorer.h.

## DataFlow

Seems to be a pull-only system to publish some game data. Will test and update the project soon(ish) (dont quote me on that timeline hahaha).

DataFlow members can also be out of an entity and out of the Entity Component system. TODO

## Game Messages

... Are 8 byte GUIDs able to be sent from the client and server and able to be observed by Statescript. Can have optional, additional data. As far as I can see other systems than Statescript can also send GameMessages.

## Components

TODO

### STUPvPGameComponent (0x24)

Contains information about the current game state, like what phase it's in, how much time is remaining, ...

Other systems access this information by using an embedded class in the GameEntityAdmin. This 

### STUStatescriptComponent (0x23)

Statescript controls:
* Ingame abilities and pretty much everything ingame except for basic movement
* User interface interactions (most of them)
* Login logic (implemented asynchronously in Statescript Actions)
* Gamemodes (Not implemented yet)
* Music
* Lobby map
* User Interfaces ingame, 2D and 3D (like the payload icon)

A Statescript component (its StatescriptSyncMgr class inside) holds several Statescript instances, which may also have a parent script ID. Script IDs are short(s). The data for a StatescriptScript is contained within a STUStatescriptGraph instance.

An implementation class is either static for the game (Actions, Conditions, Entrypoints?, ConfigVars) or initialized on demand (State). STU data and implementation nodes are associated with StatescriptRTTI (you can see the stucture in Statescript.h).

As far as I can see there is only one Entity in the LobbyEntityAdmin which holds a statescript instance. This is used for Lobby / Login UI, Login flow and Music. For ingame stuff: The controller entity has no Statescript Instance. Only the model has one which controls abilities and other various stuff. Other stuff may also have statescript instances. These include: Health Packs, Capture Points, 

### SceneRendering Component (1)

I called this SceneRendering. This component is needed if the entity wants to be shown in the map. It holds size, scaling and rotation and some other miscellanious stuff.

### MovementStateSystem (0x12, 0x15, 0x16)

(Quick note: When talking about the MovementStateSystem I mean everything which encompasses it, movement_vt, Movement system(s), Component 12 (STUMovementState), 15 (STUCharacterMoverComponent) and 16 (STUSimpleMovementComponent))

If an object needs to dynamically move in the map, the MovementStateSystem updates the entity's position, you cannot force-set the position in Component 1.

For the local player, there are some flags which you NEED to set in order for you to be able to move around. This is done by the player_spawner for you.

The most important thing is the list of MovementState in component 12. It holds all the deltas sent down from the server and does interpolation and stuff to hide network interference for you.

### Embedded / LobbyMap Entity Admin (0x54)

The embedded entity admin is very cool. Its only purpose is to display the lobby background map, and is contained within the singleton components of the Lobby Entity Admin. The component itself contains a pointer to the entity admin, 

# 3. Broken stuff and why it's broken

* "Press H to select Hero" will always be displayed once spawning a hero: This is controlled by the server by setting a boolean flag in component 
* Weapons don't shoot: I have no idea, did not look into that yet. Probably a system which needs to be explored first or a statescript var that needs to be set.
* Main Menu buttons don't work: Main menu buttons only send JAM messages to the server saying "hey i want to join a game". So that needs to be implemented
* Some abilities do not work: Some stuff does depend on the server sending you stuff and acknowledging. This is a TODO for the future.
* Practice Range does not give you hero selection screen: This is because the server spawns Tracer for you automatically. Use the Player Spawner Deluxe.
* Doors, Capture Points and Payloads get placed at (0,0,0) in the map: This is because the client by default does NOT load all entities from the map. There is a "load filter" which I bypass (see selectiveResLoad_hook in dllmain.cpp). BUT I have seen that the LobbyMap entity admin does, in fact, place them at the right positions. I dont know why this happens, TODO.
* "hashlibrary saving is disabled" message box window gets spammed: Could not save hashlibrary.json. Maybe a permission issue.
* Map unloading crashes the game: This is still a TODO. If you despawn your local hero and all the entities that were spawned by bypassing selective resource loading (see above), it will successfully unload the map without crashing. Just restart your game for now.
* Icons don't show, fonts don't properly show: Download the fonts and move them into the game directory. See Usage above.
* Lobby does not show a hero right to the buttons: TODO this is because some info is sent via JAM which you can not force modify with ViewModels.
* No weapons visible: Setting the position and modifying Statescript shortly after spawning sometimes does not work. Need to find some way to see if the hero is already spawned or not.
* DVa is floating around: I have absolutely no idea. Probably because the Mecha isnt spawned.
* Only tracer has the spawn voiceline (Cheers, Love!): VoiceLines are handled by the server and not by the client. PrometheusSystem just emulates this for Tracer, implementing this voice system is a TODO.
* Communication wheel doesnt work: See above, VoiceSystem is server-side.
* Player tablist doesnt work: Can be force-enabled in a Statescript script. Did not have time to fix this yet. (contributions welcome :D)
* "Leave game" button in login screen: Game bug
* Resizing the window screws with the game's viewmodels: Bug in the beta. Just set borderless windowed, thats the best mode for now.
* Client sometimes crashes before opening the main window: Needs fixing but happens so rarely that I wont bother rn.

# 4. Tips & Tricks

Use the up / down arrow keys to teleport yourself up/down. Use the left/right arrow keys to change the physics timescale. 

The player Spawner Deluxe can spawn a hero for you without enabling the demo. By default everything is set up so you can go ingame with Tracer. Also make sure to check out FreeLookView. It does not require you to spawn a local hero.

Make use of the entity list! It's really useful. You can resize stuff, locate stuff and make it visible / invisible using the entity bounds renderer.

Spawning multiple heroes like in the demo with multiple game instances: Enable the Demo, DP Load, load a Map, select a hero and go ingame. Then go to Tools->Awful Demo Server and start the server. Open a second instance (only tested locally) and do the same procedure. BUT connect to the server before selecting a hero (Bug, TODO). If youre interested in how this awful piece of work works, see serialization.h and state_replicator.h

You can mess around with Statescript! Go to ECS->Entity list, select a StatescriptComponent (0x23) of an entity and mess around :)

You can open the map I have shown in the demo with key M. Fun fact: The "world ping" system in Overwatch 2 exists in the beta as well! I have found an entity which gets spawned on ping, though enabling that is still a TODO. 

# 5. Contributions Welcome!
I envision a future in which we are able to play any Overwatch version that was released. With help from the community, this isn't just a dream, but a real possibility. Please help by forking, contributing, opening bug reports and sharing <3. Remember, great science is always the result of collaboration!

# 6. NOTE

Since this was hastily refactored and some types were pasted into this project which i havent checked for errors yet, some stuff may be broken which I havent noticed yet. This will get fixed in the following days / weeks.

Also dont look into the window manager. Its an abomination. You have been warned.

# 7. Open Source libraries used

(TODO, I probably forgot something)
* [keystone](https://github.com/keystone-engine/keystone)
* [capstone](github.com/capstone-engine/capstone)
* [imgui](https://github.com/ocornut/imgui/)
* [imnodes](https://github.com/BreakingBread0/imnodes/) ([my branch](https://github.com/BreakingBread0/imnodes/))
* pe (Could not find the original repository, please open an issue if you know it!)
* [lazy_importer](https://github.com/JustasMasiulis/lazy_importer)
* [nlohmann_json](https://github.com/nlohmann/json)
* [ixwebsocket](https://github.com/machinezone/IXWebSocket)
* [freetype](https://github.com/freetype/freetype)
* [Monaspace Xenon](https://monaspace.githubnext.com/) (optional)
* [Font Awesome](https://fontawesome.com) (optional)

# 8. License and Contact

MIT License. Contact me for any questions at contact@breakingbread.at or open a discussion thread <3



![prometheus logo](images/prometheus.png)