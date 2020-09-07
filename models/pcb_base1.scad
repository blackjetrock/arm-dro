//
// DRO caliper pcb mount. Holds pcb onto scale
//

ox = 57;
oy = 25;
oz = 7;

sloty = 16.1;
slotz = 4;

module pre()
{
cube([ox, oy, oz], center=true);
}

chamy = 1;
chamz = 1;

module prism(xrot)
{
    rotate([xrot, 0, 0])
    {
        difference()
        {
        cube([ox+0.1, chamy, chamz], center=true);
        translate([0, chamy/2, -chamz/2])
            {
            rotate([45, 0, 0])
                {
                remcham = sqrt(chamy*chamy + chamz*chamz);
                cube([ox+0.1, remcham, remcham], center=true);        
                }
            }
        }
    }
}

module scale_profile()
{
    difference()
    {
    cube([ox+0.1, sloty, slotz], center=true);
    translate([0, -sloty/2+chamy/2, slotz/2-chamz/2])
        {
        prism(0);
        }
 
    translate([0, sloty/2-chamy/2, slotz/2-chamz/2])
        {
        prism(270);
        }
 
    translate([0, -sloty/2+chamy/2, -slotz/2+chamz/2])
        {
        prism(90);
        }
 
    translate([0, sloty/2-chamy/2, -slotz/2+chamz/2])
        {
        prism(180);
        }
 
    }
}


module rem()
{
translate([0,0, oz/2-slotz/2+0.05])
    {
        scale_profile();
    }
}


module mount()
{
difference()
    {
    pre();
    rem();
    }
    
}

rotate([0,90,0])
mount();

