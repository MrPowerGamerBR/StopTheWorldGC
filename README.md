A simple stop-the-world mark-and-sweep GC implementation in C.

Is it good? Probably not.

For a real application I think it would be more worthwhile to add some code gen to generate all the necessary attributes for GC, like which structs participate in GC to generate their type, which fields must be walked through, so on and so forth.

...but at that point, aren't you reinventing Java?