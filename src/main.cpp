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

std::vector<double> cot_weights(TriMs& meshs,OpenMesh::VertexHandle v1, OpenMesh::VertexHandle v2, std::vector<OpenMesh::VertexHandle> vcs)
{
	auto p1 = meshs.point(v1);
	auto p2 = meshs.point(v2);
	auto w = std::vector<double>();
	for(auto p : vcs) {
		w.emplace_back(cot(p1, p2, meshs.point(p)));
	}
	return w;
}

std::vector<OpenMesh::VertexHandle> nerghbor(TriMs& meshs, OpenMesh::VertexHandle p)
{
	auto res = std::vector<OpenMesh::VertexHandle>();
	for (auto pn : meshs.voh_range(p)) {
		res.emplace_back(meshs.to_vertex_handle(pn));
	}
	return res;
}

std::vector<OpenMesh::VertexHandle> co_nerghbor(TriMs& meshs, OpenMesh::VertexHandle p1, OpenMesh::VertexHandle p2)
{
	auto res = std::vector<OpenMesh::VertexHandle>();
	for (auto p1n : meshs.voh_range(p1))
	for (auto p2n : meshs.voh_range(p2)) {
		if(meshs.to_vertex_handle(p1n).idx() == meshs.to_vertex_handle(p2n).idx() ) {
			res.emplace_back(meshs.to_vertex_handle(p1n));
		}
	}
	return res;
}

double cot_weight(TriMs& meshs, OpenMesh::VertexHandle p1, OpenMesh::VertexHandle p2)
{
	auto con = co_nerghbor(meshs, p1, p2);
	auto ws = cot_weights(meshs, p1, p2, con);
	return accumulate(ws.begin(), ws.end(), 0.0);
}

cv::Point2d laplace_cord(TriMs& meshs, OpenMesh::VertexHandle p1, std::vector<Tpt_d>& out_triplet)
{
	auto ns = nerghbor(meshs, p1);
	auto mp1 = meshs.point(p1);
	auto cp1 = cv::Point2d(mp1[0], mp1[1]);
	auto ncords = std::vector<cv::Point2d>();
	auto nweigs = std::vector<double>();
	auto ndetas = std::vector<cv::Point2d>();

	out_triplet.emplace_back(Tpt_d(p1.idx(), p1.idx(), 1));

	for(auto n : ns) {
		auto p = meshs.point(n);
		ncords.emplace_back(cv::Point2d(p[0], p[1]));
		nweigs.emplace_back(cot_weight(meshs, p1, n));
	}

	auto s = accumulate(nweigs.begin(), nweigs.end(), 0.0);
	auto x = cv::Point2d(0.0, 0.0);

	size_t num = ncords.size();

	for (size_t i = 0; i != num; ++i) {
		out_triplet.emplace_back(Tpt_d(p1.idx(), ns[i].idx(), -nweigs[i] / s));
	}
	for (size_t i = 0; i != num; ++i) {
		x += nweigs[i] * ncords[i] / s;
	}
	return cp1 - x;

}

static cv::Point2d get_point(TriMs& meshs, OpenMesh::VertexHandle const& v)
{
	return cv::Point2d(meshs.point(v)[0], meshs.point(v)[1]);
}

static cv::Point2i get_pointi(TriMs& meshs, OpenMesh::VertexHandle const& v)
{
	return cv::Point2i(static_cast<int>(meshs.point(v)[0]), static_cast<int>(meshs.point(v)[1]));
}

static cv::Point2f get_pointf(TriMs& meshs, OpenMesh::VertexHandle const& v)
{
	return cv::Point2f(static_cast<float>(meshs.point(v)[0]), static_cast<float>(meshs.point(v)[1]));
}

static auto init = false;

static auto is_fixed_vertex = std::vector<bool>();
static auto vertexsFin = std::vector<cv::Point2d>();
static auto segmentsFin = std::vector<cv::Point>();
static auto trisegsFin = std::vector<cv::Point>();
static auto laplace_cords = std::vector<cv::Point2d>();

auto delta = Eigen::MatrixX2d();
auto cache = Eigen::MatrixX2d();
auto laplace_mat = Spm_d();

static auto meshVertexs = std::vector<OpenMesh::VertexHandle>();
static auto meshFixedVertexs = std::vector<OpenMesh::VertexHandle>();
static auto meshFases = std::vector<OpenMesh::FaceHandle>(); 
static auto meshFasesOriPoints = std::vector<std::vector<cv::Point2d>>();
static auto meshFasesVhandles = std::vector<std::vector<OpenMesh::VertexHandle>>();
static auto meshs = TriMs();
static auto meshOri = TriMs();

Eigen::LeastSquaresConjugateGradient <Spm_d> lspg;

void dragPoint(int event, int x, int y, int flags, void* ustc)
{
	const auto count = meshVertexs.size();
	const auto fixed_count = std::count(is_fixed_vertex.begin(), is_fixed_vertex.end(), true);
	if (!init) {
		auto fixed_index = 0;
		auto triplet = std::vector<Tpt_d>();

		delta.resize(count + fixed_count, 2);
		laplace_mat.resize(count + fixed_count, count);

		for (size_t i = 0; i != count; ++i) {
			laplace_cords.emplace_back(laplace_cord(meshs, meshVertexs[i], triplet));
			delta(i, 0) = laplace_cords.back().x;
			delta(i, 1) = laplace_cords.back().y;
			if (is_fixed_vertex[i]) {
				meshFixedVertexs.push_back(meshVertexs[i]);
				triplet.emplace_back(Tpt_d(count + fixed_index, i, 1));
				delta(count + fixed_index, 0) = get_point(meshs, meshVertexs[i]).x;
				delta(count + fixed_index, 1) = get_point(meshs, meshVertexs[i]).y;
				++fixed_index;
			}
		}

		laplace_mat.setFromTriplets(triplet.begin(), triplet.end());

		laplace_mat.makeCompressed();
		lspg.setTolerance(0.0003);
		lspg.compute(laplace_mat);
		cache = lspg.solve(delta);

		init = true;
	}
	static auto isDragging = false;
	static size_t m = 0;
	auto pt = cv::Point2d(x, y);
	auto min = norm(pt - get_point(meshs, meshFixedVertexs[0]));

	switch (event) {
	case CV_EVENT_MOUSEMOVE:
		if (isDragging) {
			meshs.point(meshFixedVertexs[m])[0] = pt.x;
			meshs.point(meshFixedVertexs[m])[1] = pt.y;
		}

		break;
	case CV_EVENT_LBUTTONDOWN:
		for (size_t i = 0; i != meshFixedVertexs.size(); ++i)
		{
			if (min >= norm(pt - get_point(meshs, meshFixedVertexs[i]))) {
				m = i;
				min = norm(pt - get_point(meshs, meshFixedVertexs[m]));
			}
		}
		isDragging = true;
		break;
	case CV_EVENT_LBUTTONUP:
		isDragging = false;
		break;
	default:break;
	}
	if (isDragging) {
		delta(count + m, 0) = meshs.point(meshFixedVertexs[m])[0];
		delta(count + m, 1) = meshs.point(meshFixedVertexs[m])[1];

		cache = lspg.solveWithGuess(delta, cache);
		for (size_t i = 0; i != meshVertexs.size(); ++i) {
			meshs.point(meshVertexs[i])[0] = cache(i, 0);
			meshs.point(meshVertexs[i])[1] = cache(i, 1);
		}
		auto dst4 = ori.clone();
		const auto size = cv::Size(ori.cols, ori.rows);
		cv::Mat dst2;
		dst = ori.clone();

		cv::Mat black(size, src.type(), cv::Scalar::all(0));
		for (size_t i = 0; i != meshFasesVhandles.size(); ++i) {
			cv::Mat mask(size, CV_8UC1, cv::Scalar(0));
			cv::Mat mask2(size, CV_8UC1, cv::Scalar(0));

			auto ori_p = std::vector<cv::Point>{
				cv::Point(meshFasesOriPoints[i][0].x, meshFasesOriPoints[i][0].y),
				cv::Point(meshFasesOriPoints[i][1].x, meshFasesOriPoints[i][1].y),
				cv::Point(meshFasesOriPoints[i][2].x, meshFasesOriPoints[i][2].y)
			};

			auto aff_p = std::vector<cv::Point>{ 
				get_pointi(meshs, meshFasesVhandles[i][0]),
				get_pointi(meshs, meshFasesVhandles[i][1]),
				get_pointi(meshs, meshFasesVhandles[i][2])
			};

			auto ori_pf = std::vector<cv::Point2f>{
				cv::Point2f(meshFasesOriPoints[i][0].x, meshFasesOriPoints[i][0].y),
				cv::Point2f(meshFasesOriPoints[i][1].x, meshFasesOriPoints[i][1].y),
				cv::Point2f(meshFasesOriPoints[i][2].x, meshFasesOriPoints[i][2].y)
			};

			auto aff_pf = std::vector<cv::Point2f>{
				get_pointf(meshs, meshFasesVhandles[i][0]),
				get_pointf(meshs, meshFasesVhandles[i][1]),
				get_pointf(meshs, meshFasesVhandles[i][2])
			};

			std::vector<std::vector<cv::Point> >  ori_ord{
				ori_p
			};

			std::vector<std::vector<cv::Point> >  aff_ord{
				aff_p
			};

			drawContours(mask, ori_ord, 0, cv::Scalar(255), CV_FILLED, 8);
			drawContours(mask2, aff_ord, 0, cv::Scalar(255), CV_FILLED, 8);

			cv::Mat kernel(cv::Size(3, 3), CV_8UC1);
			kernel.setTo(cv::Scalar(1));
			dilate(mask, mask, kernel, cv::Point(-1, -1), 4);

			ori.copyTo(dst, mask);
			auto affTrans = getAffineTransform(ori_pf, aff_pf);
			warpAffine(dst, dst2, affTrans, size, cv::InterpolationFlags::INTER_AREA);
			dst2.copyTo(dst4, mask2);
		}
		for (size_t i = 0; i != count; ++i) {
			draw_point(dst4, cv::Point(cache(i, 0), cache(i, 1)), cv::Scalar(0, 0, 255, 0), 2);
		}
		for (size_t i = 0; i != fixed_count; ++i) {
			draw_point(dst4, get_point(meshs, meshFixedVertexs[i]), cv::Scalar(255, 0, 255, 0), 4);
		}
		for (auto a : trisegsFin) {
			draw_line(dst4, cv::Point(cache(a.x, 0), cache(a.x, 1)), cv::Point(cache(a.y, 0), cache(a.y, 1)));
		}
		imshow(mainWindowName, dst4);
	}
}

void selectPoint(int event, int x, int y, int flags, void* ustc)
{
	const auto count = meshVertexs.size();
	const auto fixed_count = std::count(is_fixed_vertex.begin(), is_fixed_vertex.end(), true);

	auto pt = cv::Point2d(x, y);
	auto min = norm(pt - get_point(meshs,meshVertexs[0]));
	auto dst4 = src.clone();
	size_t m = 0;
	switch (event) {
		case CV_EVENT_LBUTTONDOWN:
			for (size_t i = 0; i != count; ++i)
			{
				if (min >= norm(pt - get_point(meshs,meshVertexs[i]))) {
					m = i;
					min = norm(pt - get_point(meshs,meshVertexs[m]));
				}
			}
			if (min < 30) {
				is_fixed_vertex[m] = !is_fixed_vertex[m];
			}
			break;
		case CV_EVENT_RBUTTONDOWN:
			if (fixed_count < 3)break;
			cv::setMouseCallback(mainWindowName, nullptr, nullptr);
			cv::setMouseCallback(mainWindowName, dragPoint, nullptr);
			break;
		default:break;
	}
	for (size_t i = 0; i != count; ++i) {
		draw_point(
			dst4, 
			get_point(meshs,meshVertexs[i]),
			is_fixed_vertex[i]? cv::Scalar(0,255,0) : cv::Scalar(0,255,255), 
			4);
	}

	for (auto a : trisegsFin) {
		draw_line(
			dst4,
			get_point(meshs,meshVertexs[a.x]),
			get_point(meshs,meshVertexs[a.y]));
	}
	imshow(mainWindowName, dst4);
}

static int triangle_create()
{
	auto ctx = triangle_context_create();
	auto in = new triangleio();
	reset_triangleio(in);
	triangle_context_options(ctx, "pq15a2048");
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

		meshs.request_face_normals();
		meshs.request_face_status();
		meshs.request_vertex_normals();
		meshs.request_vertex_texcoords2D();
		meshs.request_vertex_status();

		meshs.request_vertex_colors();
		meshs.request_edge_status();

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
				is_fixed_vertex.emplace_back(false);
				meshVertexs.emplace_back(meshs.add_vertex(OpenMeshT::Point(vertexloop[0], vertexloop[1], 0)));
				
				setvertexmark(vertexloop, vertexnumber);
				vertexnumber++;
			}
			vertexloop = vertextraverse(m);
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

			auto facePoints = std::vector<cv::Point2d>();
			facePoints.emplace_back(cv::Point2d{ meshs.point(meshVertexs[vertexmark(p1)])[0], meshs.point(meshVertexs[vertexmark(p1)])[1] });
			facePoints.emplace_back(cv::Point2d{ meshs.point(meshVertexs[vertexmark(p2)])[0], meshs.point(meshVertexs[vertexmark(p2)])[1] });
			facePoints.emplace_back(cv::Point2d{ meshs.point(meshVertexs[vertexmark(p3)])[0], meshs.point(meshVertexs[vertexmark(p3)])[1] });
			meshFasesOriPoints.emplace_back(facePoints);

			auto face_vhandles = std::vector<OpenMesh::VertexHandle>();
			face_vhandles.emplace_back(meshVertexs[vertexmark(p1)]);
			face_vhandles.emplace_back(meshVertexs[vertexmark(p2)]);
			face_vhandles.emplace_back(meshVertexs[vertexmark(p3)]);
			meshFases.emplace_back(meshs.add_face(face_vhandles));
			meshFasesVhandles.emplace_back(face_vhandles);
			triangleloop.tri = triangletraverse(m);
			elementnumber++;
		}
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
			oriPoints.insert(oriPoints.cend(), points.begin(), points.end());
			triangle_create();
			cv::setMouseCallback(mainWindowName, selectPoint, nullptr);
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
