// DRO Milling Z slider
//
//

armz=5;
armx = 62;

clipx=5;

clipz=armz+7;
clipy=10;

m_armx = 20;
m_army = 45;
m_armz = armz;

// Thicker mount arm x
tm_x = 20;
tm_y = m_army - 15;

dropper_z = 16;
dropper_x = tm_x;
dropper_y = 10;

module cube_holed()
{
difference()
{
    cube([clipx, clipy*2, clipz], center=true);
    translate([0,0,-2])
    rotate([0, 90, 0])
    cylinder($fn=200, r=2.8/2, h=clipy*2, center=true);
}
}

module slider()
    {
       // Clip over gauge
        lenx = armx;
        translate([0,0,1.25])
        cube([lenx, clipy*2, armz+2.5], center=true);
        
        clipx_off = lenx/2+clipx/2;
        translate([clipx_off, 0, -clipz/2+armz/2])
        cube_holed();
        //cube([clipx, clipy, clipz], center=true);

        translate([-clipx_off, 0, -clipz/2+armz/2])
        cube_holed();
        //cube([clipx, clipy, clipz], center=true);

        // Arm from clip to mount plate
        translate([0, -(m_army/2+clipy/2), armz/2])
        cube([m_armx, m_army, m_armz], center=true);

        translate([0, -m_army-clipy/2+tm_y/2, 0])
        cube([tm_x, tm_y, m_armz], center=true);

        //dropper to mount plate
        translate([0, -m_army-clipy/2+dropper_y/2, -(dropper_z/2+m_armz/2)])
        cube([dropper_x, dropper_y, dropper_z], center=true);
        translate([0, -m_army-clipy/2+dropper_y/2-dropper_y, -((dropper_z-7.5)/2+m_armz/2)])
        cube([dropper_x, dropper_y, dropper_z+7.5], center=true);
        
    }

module holes()
    {
    he = 2*dropper_z+2*armz;
    translate([0, -m_army-clipy/2+dropper_y/2, 0])
        {
        cylinder($fn=200, r=3.5/2, h=he, center=true);
        }
    translate([0, -m_army-clipy/2+dropper_y/2-dropper_y, 0])
        {
        cylinder($fn=200, r=3.5/2, h=he, center=true);
        }
    }
    
rotate([180,0,0])
difference()
    {
    slider();
    holes();
    }