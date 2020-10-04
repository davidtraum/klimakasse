//Klimakasse - Halterung für Stütze

pipe_diameter = 20;
strength = 3;
height = 100;
width = pipe_diameter + strength*2;
angle = 30;
screw_size = 3;

// Die Steck-Halterung für ein 20mm PVC-Rohr
module pipe_adapter() {
    translate([pipe_diameter / 2 + strength, pipe_diameter / 2 + strength, 0]) {
        difference() {
            cylinder(d = pipe_diameter + strength * 2, h=20, $fn=100);
            translate([0,0,strength]) {
                cylinder(d = pipe_diameter, h=20 - strength, $fn=100);
            }
        }
    }
    difference() {
        cube([width,width,strength]);
        difference() {
            cube([width,width,strength]);
            translate([width/2,width/2,0]) {
                cylinder(d=width, h=strength);
            }
            translate([width/2,0,0]) {
                cube([width/2, width, strength]);
            }
        }
    }

    
}

// Der geknickte arm zur Befestigung an der Kasse
module mount_arm() {
    
    // Vorlage für ein Loch für eine Schraube
    module screw_hole() {
        rotate([0,90,0]) {
            cylinder(d=screw_size, h=strength, $fn=100);
        }
    }
    rotate([0,-30,0]) {
        cube([height, width, strength]);
        translate([height-2.5,0,strength/2]) {
            rotate([0,angle,0]) {
                difference() {
                    cube([strength,width,width]);
                    translate([0,width/4,width/2]) {
                        screw_hole();
                    }
                    translate([0,width/4*3,width/2]) {
                        screw_hole();
                    }
                }
            }
        }
    }
}


//Arm und Steckverbindung werden erzeugt.
pipe_adapter();
translate([width, 0,0]) {
    mount_arm();
}