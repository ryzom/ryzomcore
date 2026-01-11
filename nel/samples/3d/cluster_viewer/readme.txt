**************
Cluster Viewer
**************

By default the camera automatically detects the cluster
system it belongs to. In upper left corner we have the list
of cluster system in the scene (the cluster system in red is
the cluster system the camera is tested) and the cluster of
the current cluster system where the camera is.


Keys
----

Mouse - Turn view
Up/Down - Backward/Forward
Left/Right - Strafe
PgUp/PgDown - Look up/down
Shift - Speed up

L - Shows wireframe
P - Show plain polygons

A - Automatic detection of the cluster system ON/OFF
Z - Up in the list of cluster system (if autodetect off)
S - Down in the list of cluster system (if autodetect off)

TAB - Display/Hide cluster system information

Esc - Quit



Sample Specific Information
---------------------------

In the following sample some behaviour can be misunderstood.

Q: Why do I do not see the outside moving object from inside ?
A: This is because the clusters are NOT SET parent visible

Q: Why do I see all from rootCluster ?
A: This is because the clusters are SET visible from parent.

Q: Why ?
A: Because if we do not set any cluster visible from parent nor
parent visible there is no link between 'landscape' and the cluster
system.

Q: But I dont want to see the whole inside of my house from the 
landscape so what can I do ?
A: This is simple. Make a door which will be a portal to the outside.
Put the door in 2 clusters one which is inside the house and the
other outside. Flag the outside cluster with parent visible and 
visible from parent flags. And voila you've got your house indoor
no more visible from anywhere outside the house. Take care of the
portal orientation. The normal must point outside the house.

Q: Ok I have those things works but when my moving objects are inside the 
house this is like they are outside.
A: Yes because there are no automatic detection for the moving object.
You have to get this information from the PACS system and give it to
the Cluster/Portal system through the setClusterSystem function.

Q: What if my moving objects are using skeleton animation.
A: You will need to use setClusterSystem for the skeleton
only, the shape will inherit the visibility of the skeleton.

Q: ok thanks
A: you're welcome :)