# Developer Console

Debug and Release builds of SpaceGame have a built-in developer console that can be toggled via the tilde `~` key.

You can view a list of commands by submitting the `help` command.

## Available Commands

| Command    | Arguments | Description |
| -------- | ------- | ------- |
|`help`|None|Shows a list of available commands.|
|`quit`|None|Quits the game application.|
|`close`|None|Closes the developer console. You can also just press `~`.|
|`clear`|None|Clears the current console output.|
|`echo`|`<value>`|Prints `<value>` to the console.|
|`p_ShowFrameGraph`|`0` or `1`|Displays a frame graph overlay in the bottom left of the game window.|
|`p_ShowDeviceInfo`|`0` or `1`|Displays device information in the top right of the game window.|
|`p_ShowAll`|`0` or `1`|Displays all overlays (frame info, frame graph, device info).|
|`g_Pause`|None|Pauses game ticks. Doesn't pause rendering.|
|`g_Resume`|None|Resumes game ticks.|
|`g_Load`|`<name>`|Loads the given scene. Only the name needs to be provided, not the path or extension.|

> Commands that begin with `p_` are **profiler commands**.

> Commands that begin with `g_` are **game runtime commands**.
