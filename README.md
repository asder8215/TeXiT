# App Name: TeXiT
This is a simple text editor built in C, utilizing GTK4, Libadwaita, and Blueprint Compiler packages. 
The main motivation behind the text editor is purely educational.
We wanted to build an editor that, at the end, involves live editing with multiple individuals. To do it in C was an extra challenge for us.
Although live editing is not implemented into our application as of now, we still intend to develop that into our application.

# Demo of App

# Language Server (IDE)

If using `neovim` or `vscode`, clangd will need the file `compile_commands.json` to find gtk4 libraries.
To generate the file install [`bear`](https://github.com/rizsotto/bear) from your distro's package manager and run `bear -- make`.
