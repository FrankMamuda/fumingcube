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
        if (b.bondOrder === 1 && (b.stereo === ChemDoodle.structures.Bond.STEREO_PROTRUDING || b.stereo === ChemDoodle.structures.Bond.STEREO_RECESSED)) {
            bs.push(b);
        }
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

        /*
         * setLabel - called from Qt after text input
         */
        this.setLabel = function( label ) {
            // set label internally
            _.label = label;

            // find atom (label) under cursor (for editing)
            _.findHoveredObject( _.lastEvent, true, false );

            // replay stored event with the newly fetched label
            _.mouseUpAgain( _.lastEvent );
        };
    };

    let _ = states.TextInputState.prototype = new states._State();
    _.sketcher = sketcher;
    _.label = '';

    /*
     * internal mousePress event
     */
    _.innermousedown = function( e ) {
        // reset internal data
        _.downPoint = e.p;
        _.newMolAllowed = true;
        _.isMove = false;
    };

    /*
     * internal mouseRelease event
     */
    _.innermouseup = function( e ) {
        // ignore event when we're moving labels around
        if ( _.isMove )
            return;

        // store event, since we do not have the label yet
        _.lastEvent = e;

        // find atom (label) under cursor (for editing)
        // restore label if any
        _.findHoveredObject( _.lastEvent, true, false );
        if ( sketcher.hovering instanceof structures.Atom )
            _.label = sketcher.hovering.label;

        // get label from Qt
        bridge.getLabel( _.label );
    };

    /*
     * mouseUpAgain - second part of mouseRelease event
     */
    _.mouseUpAgain = function( e ) {
        _.downPoint = undefined;

        // either edit or add a new label
        if ( sketcher.hovering ) {
            sketcher.hovering.isSelected = false;
            sketcher.historyManager.pushUndo( new actions.ChangeLabelAction( sketcher.hovering, _.label ));
        } else if ( _.newMolAllowed ) {
            sketcher.historyManager.pushUndo( new actions.NewMoleculeAction( sketcher, [ new structures.Atom( _.label, e.p.x, e.p.y ) ], [] ));
        }

        // perform mouseMove event to find atom under cursor
        this.mousemove( e );
    };

    /*
     * internal mouseMove event
     */
    _.innermousemove = function( e ) {
        _.findHoveredObject( e, true, false );
    };

    /*
     * internal drag event
     */
    _.innerdrag = function( e ) {
        // disallow adding new labels when dragging
        _.newMolAllowed = false;

        // handle label dragging
        if ( sketcher.hovering ) {
            // make sure it is an atom
            if ( sketcher.hovering instanceof structures.Atom ) {
                // calculate delta
                let ps = sketcher.getMoleculeByAtom( sketcher.hovering ).atoms;
                let dif = new structures.Point( e.p.x, e.p.y );
                dif.sub( sketcher.lastPoint );

                // perform move action
                this.sketcher.historyManager.pushUndo( new actions.MoveAction( ps, dif ));
                sketcher.repaint();

                // make sure we don't make or edit labels when dragging
                _.isMove = true;
            }
        }
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
}

