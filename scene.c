#include "nano.h"

void get_scenes(scene** data) {
    scene* s;
    s = create(data, "demo1", "Heyo! This is the default scene, as defined in scene.c. Go change the content of get_scenes() to your liking after reading this demo. But by default, just press Enter to move on to the next branch.", "demo2");
    add(data, s);
    s = create(data, "demo2", "You can also make choices by entering the corresponding number. This creates branching stories, and after all, choices are a key feature of interactive fiction.", "[demo2:Read this again;demo3:Go on]");
    add(data, s);
    s = create(data, "demo3", "Looping like that wasn't a very meaningful choice, though. However, visiting a branch automatically increases its associated flag, which can be used to do some tricks...", "[?demo3=0{demo3:Brace for surprise...}?demo3=1{demo4:Surprise!}]");
    add(data, s);
    s = create(data, "demo4", "Aha! The path to the next branch was a conditional checking the branch's flag. Even a minimal parser like this can be used to create dynamic stories with a little writing skills.", "demo5");
    add(data, s);
    s = create(data, "demo5", "You can also quit and save by entering the letter 'q' at any time. The next time you start this program with any command line argument, it'll load the save file and repeat all your choices to get back to this point. So try doing that now.", "demo6");
    add(data, s);
    s = create(data, "demo6", "Pretty cool, huh? The program has minor error checking too - as long as the scenario writer didn't mess up with the parser syntax, the reader Shouldn't(tm) be able to break the program. The program ends as we try to go to an undefined branch; now go replace this demo with your own creation!", "404");
    add(data, s);
}
