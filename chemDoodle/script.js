/*
 * Copyright 2009-2020 iChemLabs, LLC.  All rights reserved.
 *
 * The ChemDoodle Web Components library is licensed under version 3
 * of the GNU GENERAL PUBLIC LICENSE.
 *
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2020 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

// DrawBridge object (via QWebChannel)
let bridge;

/*
 * marqueeTool
 */
let marqueeTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_LASSO );
    sketcher.lasso.mode = ChemDoodle.uis.tools.Lasso.MODE_RECTANGLE_MARQUEE;
    if ( !sketcher.lasso.isActive())
        sketcher.lasso.selectNextMolecule();
};

/*
 * lassoTool
 */
let lassoTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_LASSO );
    sketcher.lasso.mode = ChemDoodle.uis.tools.Lasso.MODE_LASSO;

    if ( !sketcher.lasso.isActive())
        sketcher.lasso.selectNextMolecule();
};

/*
 * benzeneTool
 */
let benzeneTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = 6;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = true;
}

/*
 * eraseTool
 */
let eraseTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_ERASE );
};

/*
 * clearTool
 */
let clearTool = function() {
    if ( sketcher.molecules.length > 0 || sketcher.shapes.length > 0 ) {
        sketcher.stateManager.getCurrentState().clearHover();

        if ( sketcher.lasso && sketcher.lasso.isActive())
            sketcher.lasso.empty();

        sketcher.historyManager.pushUndo( new ChemDoodle.uis.actions.ClearAction( sketcher ));
    }
};

/*
 * centreTool
 */
let centreTool = function() {
    let diff = new ChemDoodle.structures.Point( sketcher.width / 2, sketcher.height / 2 );
    const bounds = sketcher.getContentBounds();

    diff.x -= ( bounds.maxX + bounds.minX ) / 2;
    diff.y -= ( bounds.maxY + bounds.minY ) / 2;

    sketcher.historyManager.pushUndo( new ChemDoodle.uis.actions.MoveAction( sketcher.getAllPoints(), diff ));
};

/*
 * flipTool
 */
let flipTool = function( horizontal ) {
    let ps = sketcher.lasso.getAllPoints();
    let bs = [];
    let lbs = sketcher.lasso.getBonds();
    for ( let i = 0, ii = lbs.length; i < ii; i++ ) {
        let b = lbs[i];
        if ( b.bondOrder === 1 && ( b.stereo === ChemDoodle.structures.Bond.STEREO_PROTRUDING || b.stereo === ChemDoodle.structures.Bond.STEREO_RECESSED ))
            bs.push(b);
    }
    sketcher.historyManager.pushUndo( new ChemDoodle.uis.actions.FlipAction( ps, bs, horizontal ));
};

/*
 * flipHTool
 */
let flipHTool = function() {
    flipTool( true );
};
/*

 * flipVTool
 */
let flipVTool = function() {
    flipTool( false );
};

/*
 * zoomInTool
 */
let zoomInTool = function() {
    sketcher.styles.scale *= 1.5;
    sketcher.checkScale();
    sketcher.repaint();
};

/*
 * zoomOutTool
 */
let zoomOutTool = function() {
    sketcher.styles.scale /= 1.5;
    sketcher.checkScale();
    sketcher.repaint();
};

/*
 * solidBondTool
 */
let solidBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 1;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};

/*
 * wedgedHashedBondTool
 */
let wedgedHashedBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 1;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_RECESSED;
};

/*
 * wedgedBondTool
 */
let wedgedBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 1;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_PROTRUDING;
};

/*
 * doubleBondTool
 */
let doubleBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 2;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};

/*
 * dottedBondTool
 */
let dottedBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 0;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};

/*
 * dashedBondTool
 */
let dashedBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 0.5;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};

/*
 * wavyBondTool
 */
let wavyBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 1;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_AMBIGUOUS;
};

/*
 * doubleDashedBondTool
 */
let doubleDashedBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 1.5;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};


/*
 *
 */
/*this.buttonDoubleAmbiguous = new desktop.Button( sketcher.id + '_button_bond_ambiguous_double', imageDepot.BOND_DOUBLE_AMBIGUOUS, 'Ambiguous Double Bond', function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 2;
    sketcher.stateManager.STATE_NEW_BOND.stereo = structures.Bond.STEREO_AMBIGUOUS;
};*/


/*
 * tripleBondTool
 */
let tripleBondTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_BOND );
    sketcher.stateManager.STATE_NEW_BOND.bondOrder = 3;
    sketcher.stateManager.STATE_NEW_BOND.stereo = ChemDoodle.structures.Bond.STEREO_NONE;
};

/*
 * eraserTool
 */
let eraserTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_ERASE );
};

/*
 * cyclohexaneTool
 */
let cyclohexaneTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = 6;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = false;
};

/*
 * cyclopropaneTool
 */
let cyclopropaneTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = 3;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = false;
};

/*
 * cyclobutaneTool
 */
let cyclobutaneTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = 4;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = false;
};

/*
 * cyclopentaneTool
 */
let cyclopentaneTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = 5;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = false;
};

/*
 * varRingTool
 */
let varRingTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_RING );
    sketcher.stateManager.STATE_NEW_RING.numSides = -1;
    sketcher.stateManager.STATE_NEW_RING.unsaturated = false;
};

/*
 * chainTool
 */
let chainTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_NEW_CHAIN );
};

/*
 * getImage
 */
let getImage = function( molecules, shapes, modifySelection ) {
    // create an empty canvas
    let canvas = document.createElement( 'canvas' );
    let context = canvas.getContext( '2d' );

    // initialize bounds
    const bounds = new ChemDoodle.math.Bounds();

    // calculate bounds for contents
    function getMoleculeBounds( molecule ) {
        const mBounds = molecule.getBounds();
        bounds.minX = Math.min( mBounds.minX, bounds.minX );
        bounds.minY = Math.min( mBounds.minY, bounds.minY );
        bounds.maxX = Math.max( mBounds.maxX, bounds.maxX );
        bounds.maxY = Math.max( mBounds.maxY, bounds.maxY );
    }
    molecules.forEach( getMoleculeBounds );

    // calculate bounds for contents
    function getShapeBounds( shape ) {
        const mBounds = shape.getBounds();
        bounds.minX = Math.min( mBounds.minX, bounds.minX );
        bounds.minY = Math.min( mBounds.minY, bounds.minY );
        bounds.maxX = Math.max( mBounds.maxX, bounds.maxX );
        bounds.maxY = Math.max( mBounds.maxY, bounds.maxY );
    }
    shapes.forEach( getShapeBounds );

    // update canvas with the new size
    const width = bounds.maxX - bounds.minX;
    const height = bounds.maxY - bounds.minY;
    const scale = 4.0;
    const padding = 2.0;
    canvas.width = width * scale + padding * 2 * scale;
    canvas.height = height * scale + padding * 2 * scale;
    context.scale( scale, scale );
    context.translate( -bounds.minX + padding, -bounds.minY + padding );
    context.fillStyle = 'white';
    context.fillRect( bounds.minX - padding, bounds.minY - padding, canvas.width, canvas.height );

    // toggle atom select state (not to copy selection image)
    function toggleAtom( atom ) { if ( !modifySelection ) return; atom.isLassoed = !atom.isLassoed; }

    // paint molecules on canvas
    function drawMolecule( molecule ) {
        // deselect atoms
        let atoms = molecule.atoms;
        atoms.forEach( toggleAtom );

        // paint on canvas
        molecule.draw( context, sketcher.styles );

        // select atoms
        atoms.forEach( toggleAtom );
    }
    molecules.forEach( drawMolecule );

    // paint shapes on canvas
    function drawShape( shape ) {
        if ( modifySelection )
            shape.isLassoed = false;

        shape.draw( context, sketcher.styles );

        if ( modifySelection )
            shape.isLassoed = true;
    }
    shapes.forEach( drawShape );

    // export function
    function canvasToBase64( cnv ) { return cnv.toDataURL(); }

    // export canvas as base64 png
    return canvasToBase64( canvas );
};

/*
 * saveImageTool
 */
let saveImageTool = function() {
    const shapes = sketcher.shapes; const molecules = sketcher.molecules;
    return getImage( molecules, shapes, false );
};

/*
 * copyTool
 */
let copyTool = function() {
    if ( sketcher.lasso.isActive()) {
        // get content from lasso
        const splitter = new ChemDoodle.informatics.Splitter();
        const interpreter = new ChemDoodle.io.JSONInterpreter();
        let molecules = splitter.split( { atoms: sketcher.lasso.atoms, bonds: sketcher.lasso.getBonds() } );
        let shapes = sketcher.lasso.shapes;

        // store contents in paste buffer
        sketcher.copyPasteManager.data = interpreter.contentTo( molecules, shapes );

        // get image
        return getImage( molecules, shapes, true );
    }
};

/*
 * cutTool
 */
let cutTool = function() {
    if ( sketcher.lasso.isActive()) {
        // get content from lasso
        const splitter = new ChemDoodle.informatics.Splitter();
        const interpreter = new ChemDoodle.io.JSONInterpreter();
        let molecules = splitter.split( { atoms: sketcher.lasso.atoms, bonds: sketcher.lasso.getBonds() } );
        let shapes = sketcher.lasso.shapes;

        // get image
        let image = getImage( molecules, shapes, true );

        // cut content
        sketcher.copyPasteManager.copy( true );

        // return image
        return image;
    }
};

/*
 * pasteTool
 */
let pasteTool = function() {
    if ( sketcher.copyPasteManager.data === undefined ) {
        const json = '[_JSON]';

        // get content from json
        const interpreter = new ChemDoodle.io.JSONInterpreter();
        const { molecules, shapes } = ChemDoodle.readJSON( json );

        // store contents in paste buffer
        if ( molecules.length > 0 || shapes.length > 0 ) {
            sketcher.copyPasteManager.data = interpreter.contentTo( molecules, shapes );
            sketcher.toolbarManager.buttonPaste.enable();
            sketcher.copyPasteManager.paste();
        }
    } else {
        sketcher.copyPasteManager.paste();
    }
};

/*
 * ChangeHTMLLabelAction
 */
(function(actions, undefined) {
    'use strict';
    actions.ChangeHTMLLabelAction = function(a, after) {
        this.a = a;
        this.before = a.html;
        this.after = after;
    };
    let _ = actions.ChangeHTMLLabelAction.prototype = new actions._Action();
    _.innerForward = function() {
        this.a.setHtml( this.after );
    };
    _.innerReverse = function() {
        this.a.setHtml( this.before );
    };

})(ChemDoodle.uis.actions);


/*
 * TextInputState - a modified LabelState to allow adding textual labels to sketcher
 *                  currently we use Atom structure for labels to keep JSON compatibility
 *                  QWebChannel is asynchronous which makes things a little more complicated
 */
(function(monitor, structures, actions, states, m, undefined) {
    'use strict';

    /*
     * 'constructor'
     */
    states.TextInputState = function( sketcher ) {
        this.setup( sketcher );
    };

    let _ = states.TextInputState.prototype = new states._State();
    _.sketcher = sketcher;
    _.edit = false;
    _.hoverLabel = undefined;

    _.innermousedown = function( e ) {
        _.downPoint = e.p;
    };

    _.setLabel = function( label ) {
        if ( !label.length )
            return;

         sketcher.historyManager.pushUndo( _.edit ? new ChemDoodle.uis.actions.ChangeHTMLLabelAction( _.hoverLabel, label ) : new ChemDoodle.uis.actions.AddShapeAction( sketcher, new ChemDoodle.structures.d2.HTMLLabel( _.downPoint, label )));
        _.edit = false;
        _.hoverLabel = undefined;
    };

    _.innermouseup = function( e ) {
        _.edit = sketcher.hovering instanceof ChemDoodle.structures.d2.HTMLLabel;
        _.hoverLabel = _.edit ? sketcher.hovering : undefined;
        bridge.getLabel( _.edit ? _.hoverLabel.html : '' );
    };

    _.innermousemove = function( e ) {
        _.findHoveredObject( e, false, false, true );
    };

})(ChemDoodle.monitor, ChemDoodle.structures, ChemDoodle.uis.actions, ChemDoodle.uis.states, Math);
sketcher.stateManager.STATE_TEXT_INPUT = new ChemDoodle.uis.states.TextInputState( sketcher );

/*
 * webChannel
 */
let webChannel = new QWebChannel( qt.webChannelTransport, function( channel ) {
     bridge = channel.objects.bridge;
     bridge.labelReady.connect( sketcher.stateManager.STATE_TEXT_INPUT.setLabel );
});

/*
 * labelTool
 */
let labelTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_TEXT_INPUT );
};

/*
 * elementTool
 */
let elementTool = function( label ) {
    if ( label === '' ) {
        labelTool();
        return;
    }

    sketcher.stateManager.setState( sketcher.stateManager.STATE_LABEL );
    sketcher.stateManager.STATE_LABEL.label = label;
};

/*
 * arrowTool
 */
let arrowTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_SHAPE );
    sketcher.stateManager.STATE_SHAPE.shapeType = ChemDoodle.uis.states.ShapeState.ARROW_SYNTHETIC;
};

/*
 * retrosyntheticTool
 */
let retrosyntheticTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_SHAPE );
    sketcher.stateManager.STATE_SHAPE.shapeType = ChemDoodle.uis.states.ShapeState.ARROW_RETROSYNTHETIC;
};

/*
 * resonanceTool
 */
let resonanceTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_SHAPE );
    sketcher.stateManager.STATE_SHAPE.shapeType = ChemDoodle.uis.states.ShapeState.ARROW_RESONANCE;
};

/*
 * equilibriumTool
 */
let equilibriumTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_SHAPE );
    sketcher.stateManager.STATE_SHAPE.shapeType = ChemDoodle.uis.states.ShapeState.ARROW_EQUILIBRIUM;
};

/*
 * selectAllTool
 */
let selectAllTool = function() {
    sketcher.lasso.select( sketcher.getAllAtoms(), sketcher.shapes );
}

/*
 * exportTool
 */
let exportTool = function() {
    // get data from sketcher
    const shapes = sketcher.shapes;
    const molecules = sketcher.molecules;

    // construct new canvas and reload content
    let canvas = new ChemDoodle.ViewerCanvas( 'exportCanvas', 2, 2 );
    canvas.styles.atoms_displayTerminalCarbonLabels_2D = true;
    canvas.styles.atoms_useJMOLColors = true;
    canvas.styles.bonds_clearOverlaps_2D = true;
    canvas.styles.shapes_color = '#c10000';
    canvas.styles.backgroundColor = 'transparent';
    canvas.loadContent( molecules, shapes );

    // adjust size
    const bounds = canvas.getContentBounds();
    const width = bounds.maxX - bounds.minX;
    const height = bounds.maxY - bounds.minY;

    if ( false ) {
        const padding = 10;
        const scale = 2.0;
        canvas.resize( width * scale + padding, height * scale + padding );

        // get html canvas and upscale
        var htmlCanvas = document.getElementById( 'exportCanvas' );
        var context = htmlCanvas.getContext( '2d' );
        context.translate( -width - padding / 2, -height - padding / 2 );
        context.scale( scale, scale );
    } else {
        canvas.resize( width, height );
    }
    canvas.repaint();

    // export base64 png
    return ChemDoodle.io.png.string( canvas );
};

/*
 * positiveChargeTool
 */
let positiveChargeTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_CHARGE );
    sketcher.stateManager.STATE_CHARGE.delta = 1;
};


//(function(monitor, structures, actions, states, m, undefined) {

(function(extensions, math, structures, d2, m, undefined) {
    'use strict';
    d2.HTMLLabel = function( p, html ) {
        this.p1 = p;
        this.p2 = new ChemDoodle.structures.Point();
        this.p2.x = this.p1.x + 64;
        this.p2.y = this.p1.y + 32;
        this.setHtml( html );
        this.moveImage = new Image();
        this.moveImage.src = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAALEwAACxMBAJqcGAAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAMWSURBVEiJrZZPSKNXFMV/L/mMFJKmRaHZdFF0FR1mQNy50aCgK93oLk7rql3YdlGk0EUHCi0USgUpQhnUvWAENyooLkQqI21S1BZERQkhSMwfTDTJ93G6aJKZOjrGqWf3Du+ew73v3vce3AOSGiR57hNj3WdzNpv91uVyeYCvHtxA0uPZ2dkJy7JckpaMMRv1xLnqFLeA52tra+7V1VUDzEh698EMgG+Ojo46kskkiUSC4+Pjj4DJBzGQ9Bj4OhKJUCqVKJfLLC0tATyVNPy/DKqlkeTZ39+v8bFYDEkA05I+fCsDSQ38W4aOzc1NHMfB6/Xi9XqxbZvt7W2A94FfJZm7Mrku/kTS76ogk8nIcRyFw2GFw2HZtq1MJqNX8FzSe3dmIMmSNAH8BjzZ3d2lWCzi9/txuV5udbvd+P1+rq6uiEajAJ8Af0kKXzewXhF/BMycnZ11LCwsEIvFKJVKTE9P35qpx+NhamoKj8dDe3v7B4ODg3OSngKfGmP+BjCSrIuLi4nl5eVnKysr7ng8Tj6fB6CpqYlAIFATPDg4AKC1tbXGJRIJzs/PkURjYyOBQIBQKFQeGhr60efzPbMAksnkOycnJyaVSlEoFGrB+XyeeDxeW9u2DfAf7vLyEgBjDOVymXw+TyaTcafTaZ/P56N2+pIeSZrZ2trqWF9fZ29vj+bmZiYnX87T6OgoAHNzczVufHycdDpNMBgkFArR2dn5pzHmY2PMzms1rR6ypOLV1ZXm5+dVKBRqrVLtoioKhYIWFxdVLBYlqSzph7pu20o2L6pCtm2/ZlDlKohJ6rhJ69YBqQzaz8BnGxsbRCIRDg8PAWhpaaG/v5/e3l6AX4AvjDHlm3RuneRKwOfATldXF9lsllwuRy6XI5VK0d3dDRAFvrxN/I0GFRMbGHO73aVgMFjj29rasCzLBsaMMaU3adx5mxpjosD3AwMDWJaFZVn09fUBfHdjp7wNKt31YmRkRMPDw5L0R71vc11PpjHGljTW09Oz4ziOoY7S3MugYhI9PT39yXEc82CluY7Kt6XhPjH/AKB/6LJwN6D7AAAAAElFTkSuQmCC';
        this.editImage = new Image();
        this.editImage.src = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAWJQAAFiUBSVIk8AAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAFsSURBVEiJ7ZStbgJREIXPbBscoo/AEyBqmnTFVdiKS3gIMNukpA+AqeolgUWARK6pXwTVa4pG4CDBk0A24VTQTZZS9geoaXrk3JnvzP0b4EyRfDiXkQQ33Mn8JpwXNyH5wp9lSMrF4MPhkPP5/HImcbjrulRKsVarJZpYeeAAngGg1+vB8zzYto31eg3HcbBYLOLpDoBXknKVEW4APMXhhUIBq9UKzWYT4/EYo9EItm2jWCxGZXcAPlJ38NW5E4drreG6LjabDYwxKJfLWC6X8H0/XtoWkbdUeHSo/X6fSil2u11ut1uS5HQ6ZaVSOYhnerJp8IT4H4fff3/nnU5nD34knm1MkFQkGYYhlVJstVpnw6+TFkulEkR2v34wGMDzPFSrVdTr9SjeFpHHkw0mkwksy8JsNoPv+9Ba54KnGgRBgCAIAABaazQajVzwA0V3kKLT534Gg9zwvblN8gbA7ZHcUETeT2n8X4n6BBIVU+vQiOI5AAAAAElFTkSuQmCC';
    };

    let _ = d2.HTMLLabel.prototype = new d2._Shape();

    _.setHtml = function( html ) {
        this.image = undefined;
        this.html = html;
        this.data = 'data:image/svg+xml;base64,' + window.btoa( '<svg xmlns="http://www.w3.org/2000/svg"><foreignObject width="100%" height="100%">' + this.htmlToXML( '<div style="background: transparent; color: #000; position: absolute; height: auto; width: auto; sub { vertical-align: sub; font-size: smaller; }">' + html + '</div>' ).replace(/\#/g, '%23') + '</foreignObject></svg>' );
    }

    _.isHover = false;

    _.htmlToXML = function( html ) {
        let doc = document.implementation.createHTMLDocument( '' );
        doc.write( html );
        doc.documentElement.setAttribute( 'xmlns', doc.documentElement.namespaceURI );
        return new XMLSerializer().serializeToString( doc.body );
    }

    _.drawDecorations = function( ctx, styles ) {
        ctx.drawImage( sketcher.stateManager.getCurrentState() === sketcher.stateManager.STATE_TEXT_INPUT ? this.editImage : this.moveImage, this.p1.x + 4, this.p1.y + 4, 24, 24 );
    };

    _.draw = function(ctx, styles) {
        // draw HTML image from cache
        if ( this.image !== undefined ) {
            ctx.drawImage( this.image, this.p1.x, this.p1.y );
            return;
        }

        this.image = new Image();
        this.image.onload = function() {
            sketcher.repaint();
        };
        this.image.src = this.data;
    };

    _.getPoints = function() {
        return [ this.p1, this.p2 ];
    };

    _.isOver = function(p, barrier) {
        return math.isBetween(p.x, this.p1.x, this.p2.x) && math.isBetween(p.y, this.p1.y, this.p2.y);
    };

})(ChemDoodle.extensions, ChemDoodle.math, ChemDoodle.structures, ChemDoodle.structures.d2, Math);

/*
 * negativeChargeTool
 */
let negativeChargeTool = function() {
    sketcher.stateManager.setState( sketcher.stateManager.STATE_CHARGE );
    sketcher.stateManager.STATE_CHARGE.delta = -1;
};

/*
 * JSONInterpreter - override save/load function
 */
(function(c, io, structures, d2, JSON, undefined) {
    'use strict';
    let _ = io.JSONInterpreter.prototype;
    _.shapeTo = function( shape ) {
        let dummy = {};

        if ( shape.tmpid )
            dummy.i = shape.tmpid;

        if ( shape instanceof d2.Line ) {
            dummy.t = 'Line';
            dummy.x1 = shape.p1.x;
            dummy.y1 = shape.p1.y;
            dummy.x2 = shape.p2.x;
            dummy.y2 = shape.p2.y;
            dummy.a = shape.arrowType;
        } else if ( shape instanceof d2.HTMLLabel ) {
            dummy.t = 'HTMLLabel';
            dummy.x = shape.p1.x;
            dummy.y = shape.p1.y;
            dummy.b = window.btoa( shape.html );
        }
        return dummy;
    };

    _.shapeFrom = function(dummy, mols) {
        let shape;
        if ( dummy.t === 'Line' ) {
            shape = new d2.Line( new structures.Point( dummy.x1, dummy.y1 ), new structures.Point( dummy.x2, dummy.y2 ));
            shape.arrowType = dummy.a;
        } else if ( dummy.t === 'HTMLLabel' ) {
            const pos = new structures.Point( dummy.x, dummy.y );
            const html = dummy.b;

            if ( html.length )
                shape = new d2.HTMLLabel( pos, window.atob( dummy.b ));
        }

        return shape;
    };

})(ChemDoodle, ChemDoodle.io, ChemDoodle.structures, ChemDoodle.structures.d2, JSON);

