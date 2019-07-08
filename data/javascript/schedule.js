// By Andrzej Jan Taramina
// www.chaeron.com
// andrzej@chaeron.com
//

// Constructor for ScheduleEntry objects to hold data for all drawn objects.
// For now they will just be defined as rectangles.

function ScheduleEntry( hhmm, temp, ss) {
  //window.console.log(hhmm,temp,ss,last);
  this.ss	= ss;
  this.hhmm	= hhmm;
  this.temp	= temp;
  this.y	= this.ss.imgY;
  this.x	= this.xFromHHMM( hhmm ); 
}


// Draws this entry to a given context

ScheduleEntry.prototype.draw = function( ctx, selected ,entry) {
    //window.console.log(ctx,entry,selected);
    if( selected ) {
        ////window.console.log(this.ss.selection.temp,this.ss.tempva);
        if(this.ss.selection.temp == 0){
  	        ctx.drawImage(  this.ss.imgnoice, this.x, this.y  );
  	    }
  	    else if (this.ss.selection.temp == 1){
  	        ctx.drawImage(  this.ss.imgeco, this.x, this.y  );
  	    }
  	    else if (this.ss.selection.temp == 2){
  	        ctx.drawImage(  this.ss.imgnormal, this.x, this.y  );
  	    }
  	    else if (this.ss.selection.temp ==3){
  	        ctx.drawImage(  this.ss.imgcomfort, this.x, this.y  );
  	    }
  	    else{
  	        ctx.drawImage(  this.ss.imgnoice, this.x, this.y  );
  	        this.ss.selection.temp = 0
  	        }
        ctx.font = 'bold 10pt Arial';
	    ctx.fillStyle = "red";
  } else {
        if(entry.temp  == 0){
  	        ctx.drawImage(  this.ss.imgnoice, this.x, this.y  );
  	    }
  	    else if (entry.temp == 1){
  	        ctx.drawImage(  this.ss.imgeco, this.x, this.y  );
  	    }
  	    else if (entry.temp == 2){
  	        ctx.drawImage(  this.ss.imgnormal, this.x, this.y  );
  	    }
  	    else if (entry.temp == 3){
  	        ctx.drawImage(  this.ss.imgcomfort, this.x, this.y  );
  	    }
  	    else{
  	        ctx.drawImage(  this.ss.imgnoice, this.x, this.y  );
  	        entry.temp = 0
  	        }
	    ctx.fillStyle = "black";
	    ctx.font = '10pt Arial';
	    //ctx.drawImage(  this.ss.imgeco, this.x, this.y  );
  }

  ctx.textAlign = "center";
  ctx.fillText( this.hhmm, this.x + this.ss.imgw / 2, this.y - 20 );
  ctx.fillText( this.ss.tempva[this.temp].toFixed( 1 ), this.x + this.ss.imgw / 2, this.y - 5 );

}

// Determine if a point is inside the entry's bounds

ScheduleEntry.prototype.contains = function( mx, my ) {
  // All we have to do is make sure the Mouse X,Y fall in the area between
  // the ponters's X and ( X + Width ) and its Y and ( Y + Height )
    //window.console.log("Schedule prototype contains")
  return  ( this.x <= mx ) && ( this.x + this.ss.imgw >= mx ) &&
          ( this.y - 10 <= my ) && ( this.y + this.ss.imgh >= my );
}


// Set Entry x axis position

ScheduleEntry.prototype.setX = function( x ) {
//window.console.log("Schedule prototype setX")
  this.x = x;
  this.hhmm = this.hhmmFromX( x );
}


// Get X pos from HHMM

ScheduleEntry.prototype.xFromHHMM = function( hhmm ) {
//window.console.log("Schedule prototype xFromHHMM")
  var x;

  var parts = hhmm.split( ":" );
  var hh    = parseInt( parts[ 0 ] );
  var mm    = parseInt( parts[ 1 ] );
  
  var incr  = ( hh * 60 + mm ) / this.ss.increment;

  var rel   = incr / ( 24 * 6 );

  x = Math.ceil( this.ss.lenX * rel ) + this.ss.margin.left - this.ss.imgw / 2;

  return x;
}


// Get HHMMX pos from x

ScheduleEntry.prototype.hhmmFromX = function( x ) {
//window.console.log("Schedule prototype hhmmFromX")
  var hhmm;

  var normalized =  x - this.ss.margin.left + this.ss.imgw / 2;

  var increments = Math.floor( normalized / this.ss.lenX * 24 * 6 )

  var minutes    = increments * this.ss.increment;

  var hh		 = Math.floor( minutes / 60 );
  var mm         = minutes % 60

  var hhmm		 = ( "00" + hh ).substr( -2 ) + ":" + ( "00" + mm ).substr( -2 )

  return hhmm;
}


function ScheduleSlider( schedule, day, canvas, noiceref, ecoref, normalref,comfortref,otherSS ,giorno,tempvalue) {
  // **** First some setup! ****
  //window.console.log("ScheduleSlider---init")
  this.canvas 	= canvas;
  this.width 	= canvas.width;
  this.height 	= canvas.height;
  this.ctx 		= canvas.getContext( '2d' );
  this.margin  	= { top: 10, left: 20, right: 20, bottom: 10 };
  this.tempva   = tempvalue;
  this.schedule = schedule;
  this.day      = day;
  this.oggi		= giorno;
  this.otherSS 	= otherSS;
  
  this.imgnoice = document.getElementById( noiceref );
  this.imgeco	= document.getElementById( ecoref );
  this.imgnormal = document.getElementById( normalref );
  this.imgcomfort	= document.getElementById( comfortref );
  
  this.imgh		= this.imgnoice.height;
  this.imgw 	= this.imgnoice.width;

  this.minX 	= this.margin.left - this.imgw / 2;
  this.maxX 	= this.width - this.margin.right - this.imgw / 2 - 1;
  this.lenX 	= this.width - this.margin.left - this.margin.right;

  this.imgY 	= 45;

  this.increment = 10; 	// minutes

  this.se  		= new ScheduleEntry( "00:00", 0.0, this );

  this.entryDetails = $( "#entryDetails" );
  this.entrySlider  = $( "#slider-horizontal" );
  this.entryTime    = $( "#time" );
  this.entryTemp    = $( "#temp" );

  this.touchTimer 	= null;

  // This complicates things a little but but fixes mouse co-ordinate problems
  // when there's a border or padding. See getMouse for more detail

  var stylePaddingLeft, stylePaddingTop, styleBorderLeft, styleBorderTop;
  if( document.defaultView && document.defaultView.getComputedStyle ) {
    this.stylePaddingLeft = parseInt( document.defaultView.getComputedStyle( canvas, null )['paddingLeft'], 10 )      || 0;
    this.stylePaddingTop  = parseInt( document.defaultView.getComputedStyle( canvas, null )['paddingTop'], 10 )       || 0;
    this.styleBorderLeft  = parseInt( document.defaultView.getComputedStyle( canvas, null )['borderLeftWidth'], 10 )  || 0;
    this.styleBorderTop   = parseInt( document.defaultView.getComputedStyle( canvas, null )['borderTopWidth'], 10 )   || 0;
  }

  // Some pages have fixed-position bars at the top or left of the page
  // They will mess up mouse coordinates and this fixes that

  var html = document.body.parentNode;
  this.htmlTop = html.offsetTop;
  this.htmlLeft = html.offsetLeft;

  // **** Keep track of state! ****
  
  this.valid = false; 		// when set to false, the canvas will redraw everything
  this.entries = [];  		// the collection of things to be drawn
  this.dragging = false; 	// Keep track of when we are dragging
  							// the current selected object. In the future we could turn this into an array for multiple selection
  this.selection = null;
  this.dragoffx = 0; 		// See mousedown and mousemove events for explanation
  this.dragoffy = 0;

  
  // **** Then events! ****
  
  // This is an example of a closure!
  // Right here "this" means the ScheduleSlider. But we are making events on the Canvas itself,
  // and when the events are fired on the canvas the variable "this" is going to mean the canvas!
  // Since we still want to use this particular ScheduleSlider in the events we have to save a reference to it.
  // This is our reference!

  var thisSS = this;
  
  
  //fixes a problem where double clicking causes text to get selected on the canvas

  $( canvas ).on( 'selectstart', function( e ) { e.preventDefault(); return false; } );

  // Up, down, and move are for dragging
  $( canvas ).on( "mousedown touchstart", function( e ) {
	e.preventDefault();

	var mouse = thisSS.getMouse( e );
	var mx = mouse.x;
	var my = mouse.y;
	var entries = thisSS.entries;

	thisSS.selection = null;

	for( var i = entries.length - 1; i >= 0; i-- ) {
	  if( entries[i].contains( mx, my ) ) {
	    var mySel = entries[i];
	    // Keep track of where in the object we clicked
	    // so we can move it smoothly ( see mousemove )
	    thisSS.dragoffx = mx - mySel.x;
	    // thisSS.dragoffy = my - mySel.y;
	    thisSS.dragging = true;
	    thisSS.setSelected( mySel );
	    thisSS.valid = false;
		if( thisSS.touchTimer != null ) {
			clearTimeout( thisSS.touchTimer );
        	thisSS.touchTimer = null;
        }
		break;
	  }
	}

	if( thisSS.selection == null ) {
	  thisSS.setSelected( null ); // Make sure other SS's get deselected
	  thisSS.valid = false;

	  if( thisSS.touchTimer == null && e.type.indexOf( "touch" ) === 0 ) {
        thisSS.touchTimer = setTimeout( function () {
			thisSS.touchTimer = null;
        }, 500 )
      } else if( e.type.indexOf( "touch" ) === 0 ){
        clearTimeout( thisSS.touchTimer );
        thisSS.touchTimer = null;
        
		thisSS.handleDoubleClick( e );
      }	  
	}
  });

  $( canvas ).on( "mousemove touchmove",  function( e ) {
	e.preventDefault();

	if( thisSS.dragging ) {
	  var mouse = thisSS.getMouse( e );
	  // We don't want to drag the object by its top-left corner, we want to drag it
	  // from where we clicked. Thats why we saved the offset and use it here
	  thisSS.selection.setX( mouse.x - thisSS.dragoffx );

	  if( thisSS.selection.x < thisSS.minX ) {
		  thisSS.selection.setX( thisSS.minX );
	  } else if( thisSS.selection.x > thisSS.maxX ) {
		   thisSS.selection.setX( thisSS.maxX );
	  }

	  thisSS.entryTime.val( thisSS.selection.hhmm );

	  // thisSS.selection.y = mouse.y - thisSS.dragoffy;  // Y axis stays fixed!  
	  thisSS.valid = false; 	// Something's dragging so we must redraw
	}

  });

  
  $( canvas ).on( "mouseup touchend", function( e ) {
	e.preventDefault();
	thisSS.dragging = false;
  });


  // double click for making new entries

  $( canvas ).on( 'dblclick', function( e ) {
	thisSS.handleDoubleClick( e );
  });
  
  // **** Options! ****
  
  this.selectionColor = '#CC0000';
  this.selectionWidth = 2;  
  this.interval = 30;
  setInterval( function(  ) { thisSS.draw(  ); }, thisSS.interval );
}


ScheduleSlider.prototype.handleDoubleClick = function( e ) {
  //window.console.log("Schedule prototype handleDoubleClick")
  var mouse = this.getMouse( e );
  this.setSelected( new ScheduleEntry( this.se.hhmmFromX( mouse.x - this.margin.left + this.imgw / 2 ), 21.0, this ) );
  this.addScheduleEntry( this.selection );
}


ScheduleSlider.prototype.addScheduleEntry = function( entry ) {
//window.console.log("Schedule prototype addScheduleEntry")
  this.entries.push( entry );
  //window.console.log(this.entries[0].x);
  this.entries.sort(function(obj1,obj2){return obj1.x-obj2.x});
  //window.console.log(this.entries);
  this.valid = false;
}

ScheduleSlider.prototype.clear = function(  ) {
//window.console.log("Schedule prototype clear")
  this.ctx.clearRect( 0, 0, this.width, this.height );
}


// While draw is called as often as the INTERVAL variable demands,
// It only ever does something if the canvas gets invalidated by our code

ScheduleSlider.prototype.draw = function(  ) {
  // if our state is invalid, redraw and validate!
  //window.console.log(this.valid); 
    if( !this.valid ) {
        var ctx = this.ctx;
        var entries = this.entries;
        var l = entries.length;
        this.clear();
        this.drawDecorators( ctx );
        var point = [];
        var lastPoint;
        var altezza;
        // draw all entries
        for( var i = 0; i < l; i++ ) { 
            var entry = entries[i];
            point.push([]);
            point[i][0] = entry.temp;
            point[i][1] = entry.x;
            // We can skip the drawing of elements that have moved off the screen:
            if( entry.x > this.width || entry.y > this.height ||
                entry.x + this.imgw < 0 || entry.y + this.imgh < 0 ) continue;
                entries[i].draw( ctx, entries[ i ] == this.selection , entries[i]);
            }
        //point.sort(function(x,y){return x[1]-y[1]});
        //window.console.log("Dati : ",ctx.canvas.id,this.otherSS)
        ctx.beginPath();
        switch (ctx.canvas.id) {
            case ("heat-0"):
                lastPoint = this.otherSS[6].entries[this.otherSS[6].entries.length-1].temp;
                break;
            case ("heat-1"):
                lastPoint = this.otherSS[0].entries[this.otherSS[0].entries.length-1].temp;
                break;
            case ("heat-2"):
                lastPoint = this.otherSS[1].entries[this.otherSS[1].entries.length-1].temp;
                break;
            case ("heat-3"):
                lastPoint = this.otherSS[2].entries[this.otherSS[2].entries.length-1].temp;
                break;
            case ("heat-4"):
                lastPoint = this.otherSS[3].entries[this.otherSS[3].entries.length-1].temp;
                break;
            case ("heat-5"):
                lastPoint = this.otherSS[4].entries[this.otherSS[4].entries.length-1].temp;
                break;
            case ("heat-6"):
                lastPoint = this.otherSS[5].entries[this.otherSS[5].entries.length-1].temp;
                break;
            } 
            //window.console.log("Dati : ",lastPoint)
        switch (lastPoint) {   
            case 0:
                altezza = 78;
                ctx.lineWidth = "5";
                ctx.strokeStyle = "#0000ff";
                break;
            case 1:
                altezza = 73;
                ctx.lineWidth = "15";
                ctx.strokeStyle = "#ffff00";
                break;
            case 2:
                altezza = 68;
                ctx.lineWidth = "25";
                ctx.strokeStyle = "#00ff00";
                break;
            case 3:
                altezza = 63;
                ctx.lineWidth = "35";
                ctx.strokeStyle = "#ff0000";
                break;
            }
        
        ctx.moveTo( this.margin.left, altezza );
        ctx.lineTo( point[0][1], altezza );
        ctx.stroke(); 
        ctx.closePath();
        ctx.beginPath();
        switch (point[l-1][0]) {   
            case 0:
                altezza = 78;
                ctx.lineWidth = "5";
                ctx.strokeStyle = "#0000ff";
                break;
            case 1:
                altezza = 73;
                ctx.lineWidth = "15";
                ctx.strokeStyle = "#ffff00";
                break;
            case 2:
                altezza = 68;
                ctx.lineWidth = "25";
                ctx.strokeStyle = "#00ff00";
                break;
            case 3:
                altezza = 63;
                ctx.lineWidth = "35";
                ctx.strokeStyle = "#ff0000";
                break;
            } 
        //window.console.log(point[l-1][0],altezza);   
        ctx.moveTo( point[l-1][1]+10, altezza );
        ctx.lineTo( this.width-this.margin.right, altezza );
        ctx.stroke();
    
        for( var i = 0; i < l-1; i++ ) {
            ctx.beginPath();
            switch (point[i][0]) {   
            case 0:
                altezza = 78;
                ctx.lineWidth = "5";
                ctx.strokeStyle = "#0000ff";
                break;
            case 1:
                altezza = 73;
                ctx.lineWidth = "15";
                ctx.strokeStyle = "#ffff00";
                break;
            case 2:
                altezza = 68;
                ctx.lineWidth = "25";
                ctx.strokeStyle = "#00ff00";
                break;
            case 3:
                altezza = 63;
                ctx.lineWidth = "35";
                ctx.strokeStyle = "#ff0000";
                break;
            } 
        ctx.moveTo( point[i][1]+10, altezza );
        ctx.lineTo( point[i+1][1], altezza );
        ctx.stroke();
        //window.console.log(this.margin.left + entries[i].x,lastPoint)
        }
        ctx.closePath();
        this.valid = true;
    }
}


// Draw all the decorators (label, axis, etc.)

ScheduleSlider.prototype.drawDecorators = function( ctx ) {
	//window.console.log("Schedule prototype drawDecorators",ctx)
	// Draw Day Label
	ctx.font = 'bold 12pt Arial';
	ctx.fillStyle = "black";
	ctx.textAlign = "left";
    ctx.fillText( this.oggi, 5, 15 );
    ctx.lineWidth = "2"
	// Draw x-axis
	ctx.strokeStyle = '#FF7777';
	ctx.beginPath();
    ctx.moveTo( this.margin.left, this.imgY + 50 );
    ctx.lineTo( this.margin.left + this.lenX, this.imgY + 50 );
    ctx.stroke();
    ctx.closePath();

    // Draw x-axis labels and ticks
    var xInc =  this.lenX / 24;
    var xPos = this.margin.left;
	ctx.font = '10pt Arial';
	ctx.fillStyle = '#FF7777';
	ctx.textAlign = "center";
	for( hh = 0; hh < 24; hh++ ) {
		txt = ( "00" + hh ).substr( -2 );
        txtSize = ctx.measureText( txt );
		ctx.fillText( txt, Math.round( xPos ) + 1, this.imgY + 68 );
		ctx.beginPath();
		ctx.moveTo( Math.round( xPos ) + 1, this.imgY + 50 );
		ctx.lineTo( Math.round( xPos ) + 1, this.imgY + 45 );
		ctx.stroke();
		ctx.closePath();
		xPos += xInc;		
	}
}

// set the selected entry

ScheduleSlider.prototype.setSelected = function( sel, otherSSSelect ) {
	otherSSSelect = typeof otherSSSelect !== 'undefined' ? otherSSSelect : false;

	this.selection = sel; 
    ////window.console.log("Schedule prototype  setSelected",sel,otherSSSelect)
    if( sel ) {
        var x = parseInt(sel.ss.day);
        x = x+1;
        x = x*120;
        x = x+55;
        //window.console.log(x,sel);
        this.entryDetails.css({top: x, left: sel.ss.canvas.offsetLeft-4, position:'absolute'});
		this.entryDetails.show();
        this.entryTime.val( sel.hhmm );
		this.entrySlider.slider( "option", "value", sel.temp );
		this.entryTemp.val(this.tempva[sel.temp])//sel.temp.toFixed( 1 ) );
		this.entryDetails.data( 'selectedScheduleEntry', sel );
	} /*else {
		if( !otherSSSelect ) {
			this.entryDetails.hide();
			this.entryDetails.data( 'selectedScheduleEntry', null );
		} else {
			this.valid = false;
		}
	} */  

	if( !otherSSSelect ) {
		for( i = 0; i < this.otherSS.length; i++ ) {
			if( this.otherSS[ i ] != this ) {
				this.otherSS[ i ].setSelected( null, true );
			}
		} 
	} 
}

/// close the box
ScheduleSlider.prototype.closeSelected = function() {
    var sel = this.selection;
    //window.console.log("Chiudo il Box java ",this.otherSS,this);
    switch (this.canvas.id) {
            case ("heat-0"):
                this.otherSS[1].valid=false
                break;
            case ("heat-1"):
                this.otherSS[2].valid=false
                break;
            case ("heat-2"):
                this.otherSS[3].valid=false
                break;
            case ("heat-3"):
                this.otherSS[4].valid=false
                break;
            case ("heat-4"):
                this.otherSS[5].valid=false
                break;
            case ("heat-5"):
                this.otherSS[6].valid=false
                break;
            case ("heat-6"):
                this.otherSS[0].valid=false
                break;
            } 
    this.entryDetails.hide();
    this.entryDetails.data( 'selectedScheduleEntry', null );
    this.valid = false;
    this.setSelected( null );
    }
// delete the selected entry

ScheduleSlider.prototype.deleteSelected = function() {
	var sel = this.selection;

	if( sel ) {
		for( i = 0; i < this.entries.length; i++ ) {
			if( this.entries[ i ] == sel ) {
				this.entries.splice( i, 1 );
				break;
			}
		}

		this.setSelected( null );
	}
}


// Creates an object with x and y defined, set to the mouse position relative to the state's canvas
// If you wanna be super-correct this can be tricky, we have to worry about padding and borders

ScheduleSlider.prototype.getMouse = function( e ) {
  var element = this.canvas, offsetX = 0, offsetY = 0, mx, my;
  //window.console.log("Schedule prototype getmouse",element)
  // Compute the total offset

  if( element.offsetParent !== undefined ) {
    do {
      offsetX += element.offsetLeft;
      offsetY += element.offsetTop;
    } while( ( element = element.offsetParent ) );
  }

  // Add padding and border style widths to offset
  // Also add the <html> offsets in case there's a position:fixed bar

  offsetX += this.stylePaddingLeft + this.styleBorderLeft + this.htmlLeft;
  offsetY += this.stylePaddingTop + this.styleBorderTop + this.htmlTop;

  var pageX;
  var pageY;

  if( e.originalEvent && e.originalEvent.touches ) {
	  pageX = e.originalEvent.touches[0].pageX;
	  pageY = e.originalEvent.touches[0].pageY;
  } else {
	  pageX = e.pageX;
      pageY = e.pageY;
  }

  mx = pageX - offsetX;
  my = pageY - offsetY;
  
  // We return a simple javascript object ( a hash ) with x and y defined

  return { x: mx, y: my };
}


