# Development Log

## Contents
- [Feb 17, 2025](#feb-17-2025)

## Feb 17, 2025

I'm thinking about migrating this entire project to a Premake build system. This would simplify a lot of things and make it so that while development can continue in VS 2022, project structure isn't defined by the way VS likes to do things (something I'm *not* a fan of). 

> Cross-platform compatibility is not a consideration, so this change would be exclusively for developer convenience.

I'd project this would take a couple hours worth of work. A new branch will be created for this migration and if successful, merged back with the master branch.

Some other design considerations are the game framework itself. As of right now, I have no way of embedding the actual engine core in an editor, although I'm not too far off. The current Game class would need to be split into separate components:
- One for the actual engine
- One for the window management system (Win32)

This way, the game can be embedded in whichever window system it needs to be, regardless of whether the game owns that window or not. Nothing else should need to be refactored. 

I'm also not sold on using JSON for descriptor files, XML feels like a better fit. This would require reworking a few classes but overall would not be much of a detour. Ideally, custom binary file formats would be designed specifically for whatever use case they need to be used for, but this is not a priority or something on the docket for the near future.