nanofic
=======

A plain interactive fiction engine, originally written for NaNoWriMo use. Features include conditional branching and savestates. Change the content of scene.c to your liking.

Create a branch with create(label, content, path):

*label* is the id of the branch, which you can use to refer to it in the path parsing.  
*content* is just plain text, the actual bit of story printed to the user.  
*path* decides what comes after reading this branch, and it's a bit more tricky - there's a simple parser included that'll evaluate certain expressions and can be used for interactivity:  

"scene1" just tries to go to the branch with the label scene1 when Enter is pressed. (If it doesn't exist, the program quits, but your progress is still saved.)  
"[scene2:Go to the forest;scene3:Go to the lake]" presents two choices with different targets.  
"?scene4=0{scene5}?scene4=1{scene6}" leads you to either scene5 or scene6 depending on the flag of the branch labelled scene4.  
"[?scene7=1{scene9:Run away}?scene8=1{?scene7=1{;}scene10:Fight him}]" If you're using conditionals inside [choice braces], remember that {bracketed expressions} will be expanded literally, so you may need to add semicolons in the case of simultaneously applying conditions.

By the way, there's no error checking with the syntax. Have fun.
