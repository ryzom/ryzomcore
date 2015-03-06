//\/////
//\  overLIB Draggable Plugin
//\
//\  You may not remove or change this notice.
//\  Copyright Erik Bosrup 1998-2003. All rights reserved.
//\  Contributors are listed on the homepage.
//\  See http://www.bosrup.com/web/overlib/ for details.
//\/////
if(typeof olInfo=='undefined'||typeof olInfo.meets=='undefined'||!olInfo.meets(4.14))alert('overLIB 4.14 or later is required for the Draggable Plugin.');else{registerCommands('draggable,altcut,dragimg');
if(typeof ol_draggable=='undefined')var ol_draggable=0;if(typeof ol_altcut=='undefined')var ol_altcut=0;if(typeof ol_dragimg=='undefined')var ol_dragimg='';
var o3_draggable=0,o3_altcut=0,o3_dragimg='',olImgLeft,olImgTop,olImgObj,olMseMv;
function setDragVariables(){o3_draggable=ol_draggable;o3_altcut=ol_altcut;o3_dragimg=ol_dragimg;olImgObj=null;}
function parseDragExtras(pf,i,ar){var k=i;if(k<ar.length){if(ar[k]==DRAGGABLE){eval(pf+'draggable=('+pf+'draggable==0)?1:0');return k;}
if(ar[k]==ALTCUT){eval(pf+'altcut=('+pf+'altcut==0)?1:0');return k;}
if(ar[k]==DRAGIMG){eval(pf+'dragimg="'+ar[++k]+'"');return k;}}
return-1;}
function startDrag(){
if(o3_draggable){if(o3_sticky&&(o3_frame==ol_frame))initDrag();else o3_draggable=0;}}
function stopDrag(){if(o3_draggable)endDrag();}
function initDrag(){olMseMv=capExtent.onmousemove;if(olNs4){document.captureEvents(Event.MOUSEDOWN|Event.CLICK);document.onmousedown=grabEl;document.onclick=function(e){return routeEvent(e);}
}else{over.onmousedown=grabEl;}
if(o3_dragimg)chkForImgSupport(o3_dragimg);return true;}
function chkForImgSupport(dragImg){if(dragImg){if(typeof getAnchorObjRef!='undefined')olImgObj=getAnchorObjRef(dragImg);if(olImgObj==null)o3_dragimg='';}}
function setCursor(on){if(olNs4)return;over.style.cursor=(on?'move':'auto');}
function chkCursorPosition(Obj,XPos,YPos){if(Obj){o3_anchorx=o3_anchory=0;o3_anchoralign='UL';getAnchorLocation(Obj);if(XPos<olImgLeft||XPos>(olImgLeft+Obj.width)||YPos<olImgTop||YPos>(olImgTop+Obj.height))return false;}
return true;}
function grabEl(e){var e=(e)?e:event;var X,Y;var cKy=(olNs4?e.modifiers&Event.ALT_MASK:(!olOp?e.altKey:e.ctrlKey));if((o3_altcut?!cKy:cKy)){
X=(e.pageX||eval('e.clientX+o3_frame.'+docRoot+'.scrollLeft'));Y=(e.pageY||eval('e.clientY+o3_frame.'+docRoot+'.scrollTop'));if(chkCursorPosition(olImgObj,X,Y)){if(olNs4)document.captureEvents(Event.MOUSEUP);capExtent.onmousemove=moveEl;document.onmouseup=function(){setCursor(0);if(olIe4)over.onselectstart=null;capExtent.onmousemove=olMseMv;}
setCursor(1);if(olIe4)over.onselectstart=function(){return false;}
if(olNs4){cX=X
cY=Y
}else{
cX=X-(olNs4?over.left:parseInt(over.style.left));cY=Y-(olNs4?over.top:parseInt(over.style.top));}
return(olNs4?routeEvent(e):false);}
}else setCursor(0);}
function moveEl(e){var e=(e)?e:event;var dX,dY,X,Y;
X=(e.pageX||eval('e.clientX+o3_frame.'+docRoot+'.scrollLeft'));Y=(e.pageY||eval('e.clientY+o3_frame.'+docRoot+'.scrollTop'));if(chkCursorPosition(olImgObj,X,Y)){if(olNs4){dX=X-cX;cX=X;dY=Y-cY;cY=Y;over.moveBy(dX,dY);}else
repositionTo(over,X-cX,Y-cY);}}
function endDrag(obj){if(olNs4){document.releaseEvents(Event.MOUSEDOWN|Event.MOUSEUP|Event.CLICK);document.onmousedown=document.onclick=null;}else{if(!obj)obj=over;obj.onmousedown=null;}
document.onmouseup=null;}
registerRunTimeFunction(setDragVariables);registerCmdLineFunction(parseDragExtras);registerHook("disp",startDrag,FBEFORE);registerHook("hideObject",stopDrag,FAFTER);if(olInfo.meets(4.14))registerNoParameterCommands('draggable,altcut');}
