The original manhackcut.vtf files would repeat and stretch when applied to
a model (like w_medcrate). The solution to repeating was to set the "Clamp S"
and "Clamp T" flags using VTFEdit.  The solution to stretching was to make
the textures square.

The new manhackcut materials are only applied to models not world geometry.
