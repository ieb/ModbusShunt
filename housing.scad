$fn=100;
module Bolt(d=8,h=20) {
        cylinder(d=1.8*d,h=5,$fn=6);
        cylinder(d=d,h=h);
}
module shuntWithPCB() {
    difference() {
        union() {
        translate([0,0,0])
        cube([(105-43)/2,22,8]);
        translate([105-((105-43)/2),0,0])
        cube([(105-43)/2,22,8]);
        translate([0,2,4.5])
        cube([105,18,2]);
            translate([(105-60)/2,-4,8])
            cube([60,30,2]);
        }
        
        translate([(105-85)/2,11,-1])
        cylinder(d=8.7,h=20);
        translate([85+(105-85)/2,11,-1])
        cylinder(d=8.7,h=20);
    }

}

module fuse() {
    difference() {
        union() {
            translate([0,(19-16.2)/2,3])
            cube([68.8,16.2,2]);
            translate([(68.8-29.2)/2,0,0])
            cube([29.2,19,8]);
        }
        translate([(68.8-50.8)/2,19/2,-1])
            cylinder(d=8.7,h=20);
        translate([50.8+(68.8-50.8)/2,19/2,-1])
            cylinder(d=8.7,h=20);
    }
}

module boss() {
    difference() {
        union() {
        cylinder(d=8,h=7);
        cylinder(d=15,h=5);
        }
       translate([0,0,-1])
        cylinder(d=6.5,h=15);
    }
}
module boss2() {
    difference() {
        union() {
            cylinder(d=15,h=5);
        }
       translate([0,0,-1])
        cylinder(d=6.5,h=15);
    }
}

module bussbar1(l=50) {
    translate([-10,-10,0])
    difference() {
        union() {
            cube([l,20,2]);
        }
       translate([10,10,-1])
        cylinder(d=8.7,h=15);
       translate([l-10,10,-1])
        cylinder(d=8.7,h=15);
    }
}

module bussbar2(l=50) {
    translate([-10,-10,0])
    difference() {
        union() {
            cube([l,18,2]);
        }
       translate([9,9,-1])
        cylinder(d=8.7,h=15);
       translate([l-9,9,-1])
        cylinder(d=6.5,h=15);
    }
}

module housing() {
    difference() {
        union() {
            translate([-4,-4,0])
                cube([112,65,9]);
        }
        //fuse bolts
        translate([(68.8-50.8)/2,19/2,-0.1])
            Bolt(d=8.7);
        translate([50.8+(68.8-50.8)/2,19/2,-0.1])
            Bolt(d=8.7);
        translate([40-19+50.8+(68.8-50.8)/2,19/2,-0.1])
        cylinder(d=15.5,h=20);
        // shunt bolts
        translate([(105-85)/2,11+30,-0.1])
            Bolt(d=8.7);
        translate([85+(105-85)/2,11+30,-0.1])
            Bolt(d=8.7);
        // busbar 1
        translate([-1.5,-1,7])
        cube([21,53,5]);
        // busbar 2
        translate([49.5,-0.5,7])
        cube([41,20,5]);
        // fuse space
        translate([19,-10,4])
        cube([31,33,10]);
    }
}
module tube(d1=6,d2=3,h=31.5) {
    difference() {
  
     cylinder(d=d1,h=h);
     translate([0,0,-1])
     cylinder(d=d2,h=h+2);
    }
}
module cutaway(d=10,h=31.5) {
   cylinder(d=d,h=h);
   translate([-(d/2),-d,0])
     cube([d,d,h]);
   translate([-d,-d/2,0])
     cube([d,d,h]);
}
module cutawayshell(d1=6,d2=3,h=31.5) {
    difference() {
        union() {
        cube([d1,d1,h]);
        translate([d1/2,-d1,0])
            cube([d1,2*d1,h]);
        translate([-d1,d1/2,0])
            cube([2*d1,d1,h]);
        translate([-d1,-d1,h])
        cube([3*d1,3*d1,h]);
        }
        translate([0,0,-1])
        cube([d1/2,d1/2,h+1]);
//        cylinder(d=d1,h=h+1);
    }
}

module lid() {
    difference() {
        union() {
            difference() {
                union() {
                    translate([0,0,0])
                    cube([84+3,32+3,30+1.5]);
                    translate([0,4,0])
                    cube([84+3,28,30+1.5]);
                }
            translate([3/2,3/2,-0.1])
            cube([84,32,30]);
               translate([50,(34-22.5)/2,-1])
                cube([40,22.5,8.5]);
            }
            translate([3,3,0])
                    tube(d1=6,d2=3,h=30+1.5);
            translate([3,35-3,0])
                    tube(d1=6,d2=3,h=30+1.5);
            translate([87-3,3,0])
                    tube(d1=6,d2=3,h=30+1.5);
            translate([87-3,35-3,0])
                    tube(d1=6,d2=3,h=30+1.5);
            
            translate([3,3,10])
                    cutaway(d=10,h=31.5);
            translate([3,35-3,10])
                    rotate([0,0,-90])
                    cutaway(d=10,h=31.5);
            translate([87-3,3,10])
                    rotate([0,0,90])
                    cutaway(d=10,h=31.5);
            translate([87-3,35-3,10])
                    rotate([0,0,180])
                    cutaway(d=10,h=31.5);
            


        }
        translate([3,3,14])
                cylinder(d=7,h=30+1.5);
        translate([3,35-3,14])
                cylinder(d=7,h=30+1.5);
        translate([87-3,3,14])
                cylinder(d=7,h=30+1.5);
        translate([87-3,35-3,14])
                cylinder(d=7,h=30+1.5);
        
        translate([3,3,5])
                cylinder(d=3,h=30+1.5);
        translate([3,35-3,5])
                cylinder(d=3,h=30+1.5);
        translate([87-3,3,5])
                cylinder(d=3,h=30+1.5);
        translate([87-3,35-3,5])
                cylinder(d=3,h=30+1.5);
        
        translate([3,3,14])
                cutaway(d=7,h=31.5);
        translate([3,35-3,14])
                rotate([0,0,-90])
                cutaway(d=7,h=31.5);
        translate([87-3,3,14])
                rotate([0,0,90])
                cutaway(d=7,h=31.5);
        translate([87-3,35-3,14])
                rotate([0,0,180])
                cutaway(d=7,h=31.5);

        translate([5,5,0])
          rotate([0,0,180])
            cutawayshell(d1=10,h=31.5);

        translate([5,35-5,0])
          rotate([0,0,90])
            cutawayshell(d1=10,h=31.5);

        translate([87-5,5,0])
          rotate([0,0,-90])
            cutawayshell(d1=10,h=31.5);
        translate([87-5,35-5,0])
          rotate([0,0,0])
            cutawayshell(d1=10,h=31.5);


        
    }
    translate([0,7,0])
    cube([10,22,5]);
    
}
module parts() {

translate([0,30,0]) {
    translate([(105-85)/2,11,0])
    Bolt();
    translate([85+(105-85)/2,11,0])
    Bolt();
    translate([0,0,9])
    shuntWithPCB();
}
translate([0,0,6])
fuse();
translate([(68.8-50.8)/2,19/2,0])
Bolt();
translate([50.8+(68.8-50.8)/2,19/2,0])
Bolt();
/*
translate([50.8+(68.8-50.8)/2,19/2,9])
boss2();
*/
translate([(68.8-50.8)/2,19/2,7])
rotate([0,0,90])
bussbar1(l=52);

translate([50.8+(68.8-50.8)/2,21/2,7])
rotate([0,0,0])
bussbar2(l=40);

translate([40-19+50.8+(68.8-50.8)/2,19/2,3])
boss();
}
//translate([-4,24,9])
lid();
//parts();
//housing();

