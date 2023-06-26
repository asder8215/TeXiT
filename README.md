# App Name: TeXiT
This is a simple text editor built in C, utilizing GTK4, Libadwaita, and Blueprint Compiler packages. 
The main motivation behind the text editor is purely educational.
We wanted to build an editor that, at the end, involves live editing with multiple individuals. To do it in C was an extra challenge for us.
Although live editing is not implemented into our application as of now, we still intend to develop that into our application.

# Demo of App

# Install
Run the [install script](./install): `./install sys` to install it for all users (`/usr/local/`), or `./install user` for just your user (`~/.local/`). Make sure to have all the [build dependencies](#build-dependencies) installed.

# Build Dependencies
 - ## Blueprint
    - Builds `.blp` files into `.ui` files that can be used by GtkBuilder. Install it [here](https://jwestman.pages.gitlab.gnome.org/blueprint-compiler/setup.html)


# Language Server (IDE)

If using `neovim` or `vscode`, clangd will need the file `compile_commands.json` to find gtk4 libraries.
To generate the file install [`bear`](https://github.com/rizsotto/bear) from your distro's package manager and run `bear -- make`.
