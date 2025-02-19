# Development Log

## Contents
- [Feb 18, 2025](#feb-18-2025)
- [Feb 17, 2025](#feb-17-2025)

## Feb 18, 2025

Transition to CMake build system has been completed and merged into `master`. Descriptor files for scenes and materials have been converted from JSON to YAML for simpler editing and parsing. Scene loading and descriptor parsing has obviously been reworked due to the change in format, but also for a better designed system overall.

All third party dependencies are now fetched automatically by the build system; no need for git submodules. Hence, they have been removed. Everything else remains consistent with previous versions of the project.

Continuing work on seperating the engine core from the windowing system so work on a level editor can begin. This will likely be the following week(s) focus.

## Feb 17, 2025

I'm thinking about migrating this entire project to a Premake build system. This would simplify a lot of things and make it so that while development can continue in VS 2022, project structure isn't defined by the way VS likes to do things (something I'm *not* a fan of). 

> Cross-platform compatibility is not a consideration, so this change would be exclusively for developer convenience.

I'd project this would take a couple hours worth of work. A new branch will be created for this migration and if successful, merged back with the master branch.

Some other design considerations are the game framework itself. As of right now, I have no way of embedding the actual engine core in an editor, although I'm not too far off. The current Game class would need to be split into separate components:
- One for the actual engine
- One for the window management system (Win32)

This way, the game can be embedded in whichever window system it needs to be, regardless of whether the game owns that window or not. Nothing else should need to be refactored. 

I'm also not sold on using JSON for descriptor files, XML feels like a better fit. This would require reworking a few classes but overall would not be much of a detour. Ideally, custom binary file formats would be designed specifically for whatever use case they need to be used for, but this is not a priority or something on the docket for the near future.
