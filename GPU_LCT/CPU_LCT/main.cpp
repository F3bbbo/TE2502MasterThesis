// ------------------------------
// Geometric spaces include files

#include <tpapi_mesh.h>
#include <tpapi_polygon.h>
#include <tpapi_lct_planner.h>

// end
// ------------------------------

#include <iostream>
#include <vector>

#include "../GPU_LCT/Timer.hpp"
int main()
{
	// Activate license:
	const char* licfile = "triplannerlic.txt";
	tp_verify_license_file ( licfile ); // first check if license file is there
	tp_activate ( licfile ); // ok, load and activate

	// Path queries with clearance require a DmLct:
	// Here we create one with precision 0.001:
	tpLct* lct = tp_lct_newref ( 0.001f );

	// Initialize triangulation with domain and border expanded by 0.02:
	tp_lct_init ( lct, -100, -100, 100, 100, 0);

	// Now insert an obstacle:
	//tpPolygon* obs = tp_polygon_newref();
	//tp_polygon_read ( obs, "-3 3  -3 -3  3 -3  3 3" ); // square of side 6 centered at (0,0)
	//int id = tp_lct_insert_polygon ( lct, obs );
	//tp_lct_remove_polygon ( lct, id ); // comment this line to see the extra square


	// Obstacles can also be inserted directly from any array type of class:
	Timer t;
	t.start();

	int id;
	for (int i = 0; i < 1000; i++)
	{
		std::vector<float> poly;
		poly.push_back(-3.f + i * 0.01f); poly.push_back(-2.f + i * 0.01f);
		poly.push_back(3.f + i * 0.01f); poly.push_back(-2.f + i * 0.01f);
		poly.push_back(0.f + i * 0.01f); poly.push_back(1.f + i * 0.01f);
		id = tp_lct_insert_polygonfv(lct, poly.data(), poly.size() / 2, TpClosedPolygon);
	}
	t.stop();
	
	std::cout << t.elapsed_time() << std::endl;

	tp_lct_remove_polygon ( lct, id ); // comment out this removal and you will see the new triangle inserted

	// Obstacles can be polygonal lines as well, here are many examples:
	//# define INSERT(s) tp_polygon_read ( obs, s ); tp_lct_insert_polygon ( lct, obs )
	//INSERT ( "polyline -9 1  -9 7  -9 4  -5 4  -5 7  -5 1" ); // H
	//INSERT ( "polyline -1 1  -3 1  -3 5  -1 5  -1 3  -3 3" ); // e
	//INSERT ( "polyline 1 1 1 7" ); // l
	//INSERT ( "polyline 4 1 4 7" ); // l
	//INSERT ( "polygon 6 1  8 1  8 5  6 5" ); // o
	//INSERT ( "polyline -9 -1  -9 -7  -7 -5  -5 -7  -5 -1" ); // W
	//INSERT ( "polygon -3 -7  -1 -7  -1 -3  -3 -3" ); // o
	//INSERT ( "polyline 1 -7  1 -3  1 -4  2 -4" ); // r
	//INSERT ( "polyline 4 -7  4 -1" ); // l
	//INSERT ( "polygon 6 -7  8 -7  8 -1  8 -3  6 -3" ); // d
	//# undef INSERT

	//tp_polygon_unref ( obs ); // polygon object no more used: release it

	// Clean up:
	tp_lct_unref ( lct );
	getchar();
	return 0;
}