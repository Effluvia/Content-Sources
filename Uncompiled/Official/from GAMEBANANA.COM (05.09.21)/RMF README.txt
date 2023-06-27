
In this folder you can find the ".RMF" file with all the prefabs.
To use it, just open up Valve Hammer Editor, click "Open", and then search the
"GB_PrefabContest_309.rmf" file.
_________________________________________________________________________________________________

In the ".RMF" file there are 33 prefabs:

- 2 common lights: one hanging and moving, and one lantern with candle light effect;

- An outdoor windows-like architecture with detailed fade light effects;

- An outdoor handrail;

- An egyptian detailed column;

- An ancient wall torch with a trigger_hurt to make it realistic.

- 9 indoor furnitures and room objects: an openable wardrobe, a drawer furniture (every single
  drawer is openable), a simple door with a custom white detailed door texture, a detailed door
  jamb, white detailed double doors, a detailed radiator, a trashcan, a detailed cushion chair,
  and a ceiling rotating fan with fake floor shadow. Note: You may have to resize the shadow to
  make it realistic for different heights. You'll find the shadow in it's default size, which is
  the same as the above fan.

- A detailed 3D tiled floor. Note: Actually, every single tile is a detailed brush! You can also
  use your own (or default) textures to create your own composition that fits better in your
  level! Do NOT tie them to ANY entity if you don't want to use customized FGD's (like ZHLT or
  VHLT). Otherwise, tie them to a unique "func_wall" with the "ZHLT Lightflags" set to "Opaque
  (blocks light)" if you need to, but you may encounter some performance issues due to a lot of
  shadows to cast. And you'll have longer compile times of course. I suggest you to leave them
  simple solids to avoid glitches and performance issues. But feel free to examine that thing out
  by yourself.

- A big industrial fan with fade light effects on the grate and on the moving fan. Best shadow
  effect using VHLT, func_wall, "solid" render, "255" render FX value and "Opaque" ZHLT lightflags.

- Some little glass windows with fade light effect.

- 3 urban prefabs: a detailed seat/bench, some garbage stuff including crates, rotten wood
  pieces and dumpster with some detailed garbage sacks inside it. And a detailed street-lamp with
  a custom detailed light fade sprite;

- A detailed broken brick wall. Note: You can easily change the brick textures, if it respects
  all the faces properties and native texture size (for instance, if the actual applied texture
  is 128x128 size, you'll have to choose another texture with the same resolution and brick sizes
  to fit it properly;

- A detailed piano. Note: Do NOT tie it to a single entity with "Opaque" ZHLT Lightflag. Since
  every single piano key is a brush, like the "3D tiled floor" prefab, you may encounter the
  already mentioned issues. Anyway, examine this at your own risk;

- A very detailed pool table with custom textures, relative cues and a pool dice. Note: The pool
  table is actually an entire default func_wall. So, again, same as the piano prefab, no "Opaque"
  Lightflag. There's an invisible brush inside that already blocks the light to the floor. So
  don't worry about that;

- 5 very detailed bathroom objects with custom textures: 4 towel supports with different towel
  colours (purple, yellow, red and lime), and a toilet paper with relative support. Note: I've
  tried to apply a different tecnique to make the paper really thin, instead of using the classic
  tecnique creating a 1 pixel brush and texture it all with invisible texture but the front and
  back sides, and then turn them into a func_wall/illusionary with solid render and 255 FX amount
  value. Instead, I've just created 4 equal brushes (2 for the sides and another 2 for the corner
  fold) facing each other's perpendicular textured faces and brushes. Then, the in-game result is
  a perfectly SINGLE visible 0 pixels/units thin paper in BOTH sides with NO 1 pixel space
  between them. The paper actually kinda looks like a model instead of a brush!;

- A fire/industrial metal stairway.

- A big detailed stairway prefab for outdoors with plants, bushes, projecting rail shadows, and
  custom textures for the steps and the initial floor.

- A big detailed city sewers prefab with some custom sprites and a manhole entrance. You can even
  use it "AS IS" for a singleplayer map if you want to;

- And finally, a very nice Mayan/Egyptian style rock double doors with custom textures and opening
  doors sound. You can use it, for instance, for a temple entrance or a Mayan, Egyptian pyramid.
_________________________________________________________________________________________________

HINT: You're able to recognize every single prefab thanks to the grouped objects (no VisGroups).
_________________________________________________________________________________________________

If you're actually gone to use even only one of them (or a part of one of them), please
credit me, and also provide the GameBanana submission link on your README text file.
If you have any question, feel free to ask me on GameBanana via PM or just by adding a comment
in the submission page, or in my personal profile. I'll reply as soon as possible!

Thank you!
_________________________________________________________________________________________________

CONTACTS:

E-Mail: Alberto309@gmail.com / Steam: alberto309 ([TC]Cianez) / GameBanana: http://gamebanana.com/
_________________________________________________________________________________________________

Alberto309 aka Cianez, for the GameBanana's "Useful Props & Prefabs Contest".

21/08/2013