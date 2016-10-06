#ifdef _MSC_VER
#include <windows.h>
#endif

#include "util.h"
#include "triangle/util.h"

#include <opencv2/imgcodecs.hpp>
#include <vector>

cv::Mat src;
cv::Mat dst;
cv::Mat ori;

std::vector<cv::Point2d> points;
std::vector<cv::Point2d> oriPoints;
std::vector<imgText> labels;

double cot(OpenMeshT::Point p1, OpenMeshT::Point p2, OpenMeshT::Point pc)
{
	auto cp1 = cv::Point2d(p1[0], p1[1]);
	auto cp2 = cv::Point2d(p2[0], p2[1]);
	auto cpc = cv::Point2d(pc[0], pc[1]);
	auto l1 = cp1 - cpc;
	auto l2 = cp2 - cpc;
	auto n1 = norm(l1);
	auto n2 = norm(l2);

	l1 /= n1;
	l2 /= n2;

	auto dot = l1.ddot(l2);

	auto sqrdot = dot * dot;
	return dot / sqrt( 1 - sqrdot);
}

std::vector<double> cot_weights(TriMs& mesh,OpenMesh::VertexHandle v1, OpenMesh::VertexHandle v2, std::vector<OpenMesh::VertexHandle> vcs)
{
	auto p1 = mesh.point(v1);
	auto p2 = mesh.point(v2);
	auto w = std::vector<double>();
	for(auto p : vcs) {
		w.emplace_back(cot(p1, p2, mesh.point(p)));
	}
	return w;
}

std::vector<OpenMesh::VertexHandle> nerghbor(TriMs& mesh, OpenMesh::VertexHandle p)
{
	auto res = std::vector<OpenMesh::VertexHandle>();
	for (auto pn : mesh.voh_range(p)) {
		res.emplace_back(mesh.to_vertex_handle(pn));
	}
	return res;
}

std::vector<OpenMesh::VertexHandle> co_nerghbor(TriMs& mesh, OpenMesh::VertexHandle p1, OpenMesh::VertexHandle p2)
{
	auto res = std::vector<OpenMesh::VertexHandle>();
	for (auto p1n : mesh.voh_range(p1))
	for (auto p2n : mesh.voh_range(p2)) {
		if(mesh.to_vertex_handle(p1n).idx() == mesh.to_vertex_handle(p2n).idx() ) {
			res.emplace_back(mesh.to_vertex_handle(p1n));
		}
	}
	return res;
}

double cot_weight(TriMs& mesh, OpenMesh::VertexHandle p1, OpenMesh::VertexHandle p2)
{
	auto con = co_nerghbor(mesh, p1, p2);
	auto ws = cot_weights(mesh, p1, p2, con);
	return accumulate(ws.begin(), ws.end(), 0.0);
}

cv::Point2d laplace_cord(TriMs& mesh, OpenMesh::VertexHandle p1, std::vector<Tpt_d>& out_triplet)
{
	auto ns = nerghbor(mesh, p1);
	auto mp1 = mesh.point(p1);
	auto cp1 = cv::Point2d(mp1[0], mp1[1]);
	auto ncords = std::vector<cv::Point2d>();
	auto nweigs = std::vector<double>();
	auto ndetas = std::vector<cv::Point2d>();

	out_triplet.emplace_back(Tpt_d(p1.idx(), p1.idx(), 1));

	for(auto n : ns) {
		auto p = mesh.point(n);
		ncords.emplace_back(cv::Point2d(p[0], p[1]));
		nweigs.emplace_back(cot_weight(mesh, p1, n));
	}

	auto s = accumulate(nweigs.begin(), nweigs.end(), 0.0);
	auto x = cv::Point2d(0.0, 0.0);

	size_t num = ncords.size();

	for (size_t i = 0; i != num;++i) {
		out_triplet.emplace_back(Tpt_d(p1.idx(), ns[i].idx(), -nweigs[i]/s));
	}

	for (size_t i = 0; i != num;++i) {
		x += nweigs[i] * ncords[i] / s;
	}

	//for (size_t i = 0; i != num; ++i) {
	//	out_triplet.emplace_back(Tpt_d(p1.idx(), ns[i].idx(), -1.0 / num));
	//}
	
	// x = cv::Point2d(0.0, 0.0);
	//for (size_t i = 0; i != num; ++i) {
	//	x += ncords[i] / static_cast<double>(num);
	//}

	return cp1 - x;

}

auto vertexsFin = std::vector<cv::Point2d>();
auto segmentsFin = std::vector<cv::Point>();
auto trisegsFin = std::vector<cv::Point>();

static int triangle_create()
{
	auto ctx = triangle_context_create();
	auto in = new triangleio();
	reset_triangleio(in);
	triangle_context_options(ctx, "pq15a256");
	in->numberofsegments = oriPoints.size();
	in->numberofpoints = oriPoints.size();

	in->segmentlist = static_cast<int *>(malloc(in->numberofsegments * 2 * sizeof(int)));
	in->segmentmarkerlist = static_cast<int *>(malloc(in->numberofsegments * sizeof(int)));


	in->pointlist = static_cast<REAL *>(malloc(in->numberofpoints * 2 * sizeof(REAL)));
	in->pointmarkerlist = static_cast<int *>(malloc(in->numberofpoints * sizeof(int)));

	for (size_t i = 0; i != in->numberofsegments; ++i) {

		in->pointlist[i * 2] = oriPoints[i].x;
		in->pointlist[i * 2 + 1] = oriPoints[i].y;
		in->pointmarkerlist[i] = 2;

		in->segmentlist[i * 2] = i + 1;
		in->segmentlist[i * 2 + 1] = (i + 1) % in->numberofsegments + 1;

		in->segmentmarkerlist[i] = 2;
	}

	auto res = triangle_mesh_create(ctx, in);

	if (!res) {
		auto mesh = TriMs();
		auto meshVertexs = std::vector<OpenMesh::VertexHandle>();
		auto meshFases = std::vector<OpenMesh::FaceHandle>();

		mesh.request_face_normals();
		mesh.request_face_status();
		mesh.request_vertex_normals();
		mesh.request_vertex_texcoords2D();
		mesh.request_vertex_status();

		mesh.request_vertex_colors();
		mesh.request_edge_status();

		struct osub subsegloop;
		vertex endpoint1, endpoint2;
		long subsegnumber;

		auto m = ctx->m;
		auto b = ctx->b;

		vertex vertexloop;
		int vertexnumber;

		dst = ori.clone();

		traversalinit(&m->vertices);
		vertexnumber = 0;
		vertexloop = vertextraverse(m);
		while (vertexloop != static_cast<vertex>(nullptr)) {
			if (!b->jettison || vertextype(vertexloop) != UNDEADVERTEX) {
				vertexsFin.emplace_back(cv::Point2d{ vertexloop[0], vertexloop[1] });

				meshVertexs.emplace_back(mesh.add_vertex(OpenMeshT::Point(vertexloop[0], vertexloop[1], 0)));
				
				setvertexmark(vertexloop, vertexnumber);
				vertexnumber++;
			}
			vertexloop = vertextraverse(m);
		}

		for (auto p : vertexsFin) {
			draw_point(dst, p , cv::Scalar(0, 200, 0, 0));
		}

		traversalinit(&m->subsegs);
		subsegloop.ss = subsegtraverse(m);
		subsegloop.ssorient = 0;
		subsegnumber = b->firstnumber;
		while (subsegloop.ss != static_cast<subseg *>(nullptr)) {
			sorg(subsegloop, endpoint1);
			sdest(subsegloop, endpoint2);
			segmentsFin.emplace_back(cv::Point{ vertexmark(endpoint1), vertexmark(endpoint2) });
			subsegloop.ss = subsegtraverse(m);
			subsegnumber++;
		}
		
		struct otri triangleloop;
		vertex p1, p2, p3;
		long elementnumber;

		int plus1mod3[3] = { 1, 2, 0 };
		int minus1mod3[3] = { 2, 0, 1 };

		traversalinit(&m->triangles);
		triangleloop.tri = triangletraverse(m);
		triangleloop.orient = 0;
		elementnumber = b->firstnumber;
		while (triangleloop.tri != static_cast<triangle *>(nullptr)) {
			org(triangleloop, p1);
			dest(triangleloop, p2);
			apex(triangleloop, p3);
			trisegsFin.emplace_back(cv::Point{ vertexmark(p1), vertexmark(p2) });
			trisegsFin.emplace_back(cv::Point{ vertexmark(p2), vertexmark(p3) });
			trisegsFin.emplace_back(cv::Point{ vertexmark(p3), vertexmark(p1) });

			auto face_vhandles = std::vector<OpenMesh::VertexHandle>();
			face_vhandles.emplace_back(meshVertexs[vertexmark(p1)]);
			face_vhandles.emplace_back(meshVertexs[vertexmark(p2)]);
			face_vhandles.emplace_back(meshVertexs[vertexmark(p3)]);
			mesh.add_face(face_vhandles);
			triangleloop.tri = triangletraverse(m);
			elementnumber++;
		}
		
		auto laplace_cords = std::vector<cv::Point2d>();

		const auto num = meshVertexs.size();

		auto delta_m = cv::Mat_<double>(cv::Size(2, num + 3), 0);

		auto ans_m = cv::Mat_<double>(cv::Size(2, num + 3), 0);

		auto t_m = cv::Mat_<double>(cv::Size(num, num + 3),0);

		for(auto i = 0;i != num; ++i) {
			t_m[meshVertexs[i].idx()][meshVertexs[i].idx()] = 1.0;
			auto triplet = std::vector<Tpt_d>();
			laplace_cords.emplace_back(laplace_cord(mesh, meshVertexs[i], triplet));
			delta_m[meshVertexs[i].idx()][0] = laplace_cords.back().x;
			delta_m[meshVertexs[i].idx()][1] = laplace_cords.back().y;
			for(auto tr : triplet) {
				t_m[meshVertexs[std::get<0>(tr)].idx()][meshVertexs[std::get<1>(tr)].idx()] = std::get<2>(tr);
			}
		}

		t_m[num + 0][meshVertexs[0].idx()] = 1;
		t_m[num + 1][meshVertexs[1].idx()] = 1;
		t_m[num + 2][meshVertexs[2].idx()] = 1;

		delta_m[num + 0][0] = mesh.point(meshVertexs[0])[0];
		delta_m[num + 0][1] = mesh.point(meshVertexs[0])[1];
		delta_m[num + 1][0] = mesh.point(meshVertexs[1])[0];
		delta_m[num + 1][1] = mesh.point(meshVertexs[1])[1];
		delta_m[num + 2][0] = mesh.point(meshVertexs[2])[0];
		delta_m[num + 2][1] = mesh.point(meshVertexs[2])[1];

		solve(t_m.t() * t_m, t_m.t() * delta_m, ans_m);

		cv::namedWindow("x", 1);
		imshow("x", dst);

		triangle_context_destroy(ctx);

		free(in->segmentlist);
		free(in->segmentmarkerlist);

		free(in->pointlist);
		free(in->pointmarkerlist);

		delete in;
	}

	return res;
}

static void mouse_event_handle(int event, int x, int y, int flags, void* ustc)
{
	cv::Point pt;
	std::string temp;
	static int n = 0;
	auto clrPoint = cv::Scalar(255, 0, 0, 0);
	auto clrText = cv::Scalar(255, 200, 0, 0);
	if (event == CV_EVENT_MOUSEMOVE) {
		src = dst.clone();

		x = bound(x, 0, src.cols - 1);
		y = bound(y, 0, src.rows - 1);
		pt = cv::Point(x, y);

		if (points.size() >= 3 && if_in_range(pt, points)) {
			pt = points[0];
			temp = string_format("%s", "");
		}
		else {
			temp = string_format("%d (%d,%d)", n + 1, pt.x, pt.y);
		}

		draw_text(src, temp, pt, get_inverseColor(clrText));
		draw_point(src, pt, get_inverseColor(clrPoint));

		for (auto p : points) {
			draw_point(src, p, clrPoint);
		}

		for (auto l : labels) {
			draw_text(src, l);
		}

		if (points.size() > 1) {
			for (size_t i = 1; i != points.size(); ++i)
			{
				draw_line(src, points[i - 1], points[i]);
			}
		}

		if (points.size() >= 1) {
			draw_line(src, points[points.size() - 1], pt);
		}
		imshow(mainWindowName, src);
	} else if (event == CV_EVENT_LBUTTONDOWN) {

		pt = cv::Point(x, y);

		if (points.size() >= 3 && if_in_range(pt, points)) {
			imshow(mainWindowName, src);
			cv::setMouseCallback(mainWindowName, nullptr, nullptr);
			//cv::setMouseCallback(mainWindowName, drag_point, nullptr);
			oriPoints.insert(oriPoints.cend(), points.begin(), points.end());
			//addPoint();
			triangle_create();
		} else {
			points.push_back(pt);
			temp = string_format("%d (%d,%d)", n, pt.x, pt.y);
			labels.emplace_back(imgText(temp, pt, clrText));
			n++;
		}
	}else if (event == CV_EVENT_RBUTTONDOWN) {
		if (!points.empty()) {
			points.pop_back();
			labels.pop_back();
		}
	}
}

#define _DEBUG
#ifdef _MSC_VER
#ifdef _DEBUG
int main()
#else
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
#endif // !DEBUG

#elif
int main()
#endif// _MSC_VER
{

	cv::namedWindow(mainWindowName, 1);

	ori = cv::imread("qwe.jpg");
	src = dst = ori.clone();

	imshow(mainWindowName, src);
	cv::setMouseCallback(mainWindowName, mouse_event_handle, nullptr);

	while(cv::waitKey(0) != 27) {
		
	}

	cv::destroyAllWindows();


	return 0;
}