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
const padding = 10;
const scale = 2.0;
canvas.resize( width * scale + padding, height * scale + padding );

// get html canvas and upscale
var htmlCanvas = document.getElementById( 'exportCanvas' );
var context = htmlCanvas.getContext( '2d' );
context.translate( -width - padding / 2, -height - padding / 2 );
context.scale( scale, scale );
canvas.repaint();

// export base64 png
ChemDoodle.io.png.string( canvas );
