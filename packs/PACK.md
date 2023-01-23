# texture pack system

the texture pack system allows for custom textures to be used in the game.  
to create a texture pack, make a new folder with `textures.toml` in its root.  

`textures.toml`  
```toml
[meta]
name = "PACK NAME"
author = "PACK AUTHOR"
card_vertical_spacing = # float, how far apart tableu cards should be

# List of spritesheets required for your texture packs.  Note that
# spritesheets are not required for
[[spritesheets]]
name = "spritesheet_name"
texture = "path/to/texture.png"
dimensions = [rows, columns]

# By setting name = "Default", these items are in the menu under Pack Name.
# For variants, set a name.  This will add another option Pack Name | Name 
# in the menu.  Note that every face card must be defined under default as 
# we fallback to the Default textures when a named variant is missing some
# card.

[[backgrounds]]
  name = "Default"
  type = # one of "colour", "cover", "stretch", "tiled"
  texture = "path/to/background.png" # if type != "colour"
  colour = "#ff0000" # if type == "colour"
  placeholder = "#ffffff" # colour to render empty pile outline

[cards]
  # Here you define the textures that cards use.  Acceptable subkeys are
  # 'backs', 'aces', 'ones', ..., 'queens', 'kings', 'clubs', 'hearts',
  # 'spades', 'diamonds', or individual cards ('A_diamonds', '3_hearts', etc)
```

See the packs in this folder for more examples.