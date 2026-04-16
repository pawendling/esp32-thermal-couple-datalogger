$fn = 60;

// -----------------------------------------
// Parameter bundle
// -----------------------------------------
function cfg() = [
    ["base_x", 7],
    ["base_y", 7],
    ["base_z", 1.25],

    ["pcb_x", 20.1],
    ["pcb_y", 23.1],

    ["holder_t", 2],
    ["holder_l", 4],
    ["hh", 4.5],
    ["tol", 0.7],

    ["overh", 1],
    ["overt", 1]
];

// -----------------------------------------
// Key/value lookup
// -----------------------------------------
function get(c, key) =
    let(idx = search([key], [for (i=c) i[0]])[0])
        (idx == -1 ? undef : c[idx][1]);

// -----------------------------------------
// Clip modules
// -----------------------------------------
module clipR(c) {
    holder_t = get(c,"holder_t");
    holder_l = get(c,"holder_l");
    base_z = get(c,"base_z");
    hh = get(c,"hh");
    overh = get(c,"overh");
    overt = get(c,"overt");

    cube([holder_t, holder_l, base_z + hh]);
    translate([-overh, 0, base_z + hh - overt])
        cube([holder_t + overh, holder_l, overt]);
}

module clipL(c) {
    holder_t = get(c,"holder_t");
    holder_l = get(c,"holder_l");
    base_z = get(c,"base_z");
    hh = get(c,"hh");
    overh = get(c,"overh");
    overt = get(c,"overt");

    cube([holder_t, holder_l, base_z + hh]);
    translate([0, 0, base_z + hh - overt])
        cube([holder_t + overh, holder_l, overt]);
}

// -----------------------------------------
// PCB Holder
// -----------------------------------------
module pcb_holder(c) {

    pcb_x = get(c,"pcb_x");
    pcb_y = get(c,"pcb_y");
    holder_t = get(c,"holder_t");
    holder_l = get(c,"holder_l");
    base_z = get(c,"base_z");
    hh = get(c,"hh");
    tol = get(c,"tol");

    // Wire guide
    translate([holder_t + pcb_x/2 - 4, pcb_y + 3*holder_t, 0])
    difference() {
        cube([8, 3, base_z + 8.5]);
        translate([4,4,base_z + 5])
            rotate([90,0,0]) cylinder(40, d=3.2);
    }

    // Center peg
    translate([holder_t + pcb_x/2 + tol/2,
               pcb_y/2 + holder_t + tol/2, 0])
        cylinder(base_z + 1.5, d=5);

    // Top holders
    translate([holder_t*1.5, pcb_y + holder_t + tol, 0]) {
        cube([holder_l, holder_t, base_z + hh]);
        translate([pcb_x * .7 + tol, 0, 0])
            cube([holder_l, holder_t, base_z + hh]);
    }

    // Upper L/R holders
    translate([0, pcb_y*.95 - holder_l + holder_t + tol, 0]) {
        clipL(c);
        translate([holder_t + pcb_x + tol, 0, 0])
            clipR(c);
    }

    // Lower L/R holders
    translate([0, holder_t + pcb_y*0.05, 0]) {
        clipL(c);
        translate([holder_t + pcb_x + tol, 0, 0])
            clipR(c);
    }

    // Bottom holder
    translate([pcb_x/2, 0, 0])
        cube([5, holder_t, base_z + hh]);
}

// -----------------------------------------
// Base + Holder Assembly
// -----------------------------------------
module pcb_base(c) {
    
    // Numer of cards 
    //
    //
    n = 4; 
    
    base_x = get(c,"base_x");
    base_y = get(c,"base_y");
    base_z = get(c,"base_z");
    pcb_x = get(c,"pcb_x");
    pcb_y = get(c,"pcb_y");
    holder_t = get(c,"holder_t");
    tol = get(c,"tol");
    
   
    cardx = pcb_x + holder_t + tol;
    marginx = (base_x - (n*cardx))/2; 
    
    
    cx = pcb_x/2 + holder_t + tol/2;
    cy = pcb_y/2 + holder_t + tol/2;

    for(i = [0 : n-1 ])
       translate([(base_x/2)+ (i * cardx), base_y/2])
          pcb_holder(c);
    
    // Base assembly 
   
        // Compute final dimensions
    X = base_x + (n*(pcb_x + holder_t + tol)) + holder_t;
    Y = base_y + pcb_y + (2*holder_t) + tol + 3*holder_t;
    Z = base_z;

    hole_d = 3;          // 2 mm hole
    inset  = 3.5;          // distance from edges (adjust as needed)

    difference() {
        cube([X, Y, Z]);

        // Four corner holes
        translate([inset, inset, 0])
            cylinder(h = Z + 1, d = hole_d);

        translate([X - inset, inset, 0])
            cylinder(h = Z + 1, d = hole_d);

        translate([inset, Y - inset, 0])
            cylinder(h = Z + 1, d = hole_d);

        translate([X - inset, Y - inset, 0])
            cylinder(h = Z + 1, d = hole_d);
    }

   

}

// -----------------------------------------
// Render
// -----------------------------------------
pcb_base(cfg());
