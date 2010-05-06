//\/////
//\  overLIB Draggable Plugin
//\
//\  You may not remove or change this notice.
//\  Copyright Erik Bosrup 1998-2003. All rights reserved.
//\  Contributors are listed on the homepage.
//\  See http://www.bosrup.com/web/overlib/ for details.
//\/////
////////
// PRE-INIT
// Ignore these lines, configuration is below.
////////
if (typeof olInfo == 'undefined' || typeof olInfo.meets == 'undefined' || !olInfo.meets(4.14)) alert('overLIB 4.14 or later is required for the Draggable Plugin.');
else {
registerCommands('draggable,altcut,dragimg');
////////
// DEFAULT CONFIGURATION
// Settings you want everywhere are set here. All of this can also be
// changed on your html page or through an overLIB call.
////////
if (typeof ol_draggable=='undefined') var ol_draggable=0;
if (typeof ol_altcut=='undefined') var ol_altcut=0;
if (typeof ol_dragimg=='undefined') var ol_dragimg='';
////////
// END OF CONFIGURATION
// Don't change anything below this line, all configuration is above.
////////
////////
// INIT
////////
// Runtime variables init. Don't change for config!
var o3_draggable=0;
var o3_altcut=0;
var o3_dragimg='';
var olImgLeft,olImgTop;
var olImgObj;
var olMseMv;  // hold old mouseMove routine
////////
// PLUGIN FUNCTIONS
////////
function setDragVariables() {
	o3_draggable=ol_draggable;
	o3_altcut=ol_altcut;
	o3_dragimg=ol_dragimg;
	olImgObj=null;
}
// Parses Draggable commands
function parseDragExtras(pf,i,ar) {
	var k=i;
	if (k < ar.length) {
		if (ar[k]==DRAGGABLE) { eval(pf+'draggable=('+pf+'draggable==0) ? 1 : 0'); return k; }
		if (ar[k]==ALTCUT) { eval(pf+'altcut=('+pf+'altcut==0) ? 1 : 0'); return k; }
		if (ar[k]==DRAGIMG) { eval(pf+'dragimg="'+ar[++k]+'"'); return k; }
	}
	return -1;
}
//////
//  PRESHOW PROCESSING FOR DRAGGABLE POPUPS
//////
function startDrag() {
 // Initiate dragging if in same frame and its a sticky
	if (o3_draggable) {
		if (o3_sticky&&(o3_frame==ol_frame)) initDrag();
		else o3_draggable=0;
	}
}
//////
//  POSTHIDE PROCESSING FOR DRAGGABLE POPUPS
//////
function stopDrag() {
	if (o3_draggable) endDrag();
}
//////
// DRAGGABLE FUNCTIONS
//////
function initDrag() {
	olMseMv=capExtent.onmousemove;
	if(olNs4) {
		document.captureEvents(Event.MOUSEDOWN | Event.CLICK);
		document.onmousedown=grabEl;
		document.onclick=function(e) {return routeEvent(e);}
	} else {
		over.onmousedown=grabEl;
	}
	if (o3_dragimg) chkForImgSupport(o3_dragimg);
	return true;
}
// Checks for image for dragging
function chkForImgSupport(dragImg) {
	if (dragImg) {
		if (typeof getAnchorObjRef!='undefined') olImgObj=getAnchorObjRef(dragImg);
		if (olImgObj==null) o3_dragimg='';
	}
}
// Sets cursor symbol
function setCursor(on) {
	if (olNs4) return;
	over.style.cursor=(on ? 'move' : 'auto');
}
// Checks cursor position relative to image
function chkCursorPosition(Obj,XPos,YPos) {
	if (Obj) {
		o3_anchorx=o3_anchory=0;
		o3_anchoralign='UL';
		getAnchorLocation(Obj);
		if (XPos < olImgLeft||XPos > (olImgLeft+Obj.width)||YPos < olImgTop||YPos > (olImgTop+Obj.height)) return false;
	}
	return true;
}
// Sets up mouse grab for moving
function grabEl(e) {
	var e=(e) ? e : event;
	var X,Y;
	var cKy=(olNs4 ? e.modifiers & Event.ALT_MASK : (!olOp ? e.altKey : e.ctrlKey));
	if ((o3_altcut ? !cKy : cKy)) {
		//   get mouse's current x,y location
		X=(e.pageX || eval('e.clientX+o3_frame.'+docRoot+'.scrollLeft'));
		Y=(e.pageY || eval('e.clientY+o3_frame.'+docRoot+'.scrollTop'));
	  if (chkCursorPosition(olImgObj,X,Y)) {
	 		if (olNs4) document.captureEvents(Event.MOUSEUP);
	 		capExtent.onmousemove=moveEl;
	 		document.onmouseup=function() {setCursor(0); if (olIe4) over.onselectstart=null; capExtent.onmousemove=olMseMv;}
	 		setCursor(1);
	 		if (olIe4) over.onselectstart=function() {return false;}
	 		if (olNs4) {
	  		cX=X
	  		cY=Y
	 		} else {
	  		// get offsets from upper left hand corner of popup to keep popup from jummping
	  		// when first starting to drag
	  		cX=X-(olNs4 ? over.left : parseInt(over.style.left));
	  		cY=Y-(olNs4 ? over.top : parseInt(over.style.top)); 
	 		}
	 		return (olNs4 ? routeEvent(e) : false);
	  }
	} else setCursor(0);
}
// Moves popup to follow mouse
function moveEl(e) {
	var e=(e) ? e : event;
	var dX,dY,X,Y;
	//  get new mouse location
	X=(e.pageX || eval('e.clientX+o3_frame.'+docRoot+'.scrollLeft'));
	Y=(e.pageY || eval('e.clientY+o3_frame.'+docRoot+'.scrollTop'));
	if (chkCursorPosition(olImgObj,X,Y)){
		if (olNs4) {
			dX=X-cX; cX=X;
			dY=Y-cY; cY=Y;
			over.moveBy(dX,dY);
		} else 
			repositionTo(over,X-cX,Y-cY);  // move popup to that position
	}
}
// Cleanup for Drag end
function endDrag(obj) {
	if (olNs4) {
		document.releaseEvents(Event.MOUSEDOWN | Event.MOUSEUP | Event.CLICK);
		document.onmousedown=document.onclick=null;
	} else {
		if(!obj) obj=over;
		obj.onmousedown=null;
	}
	document.onmouseup= null;
}
////////
// PLUGIN REGISTRATIONS
////////
registerRunTimeFunction(setDragVariables);
registerCmdLineFunction(parseDragExtras);
registerHook("disp",startDrag,FBEFORE);
registerHook("hideObject",stopDrag,FAFTER);
if (olInfo.meets(4.14)) registerNoParameterCommands('draggable,altcut');
}
//end 
