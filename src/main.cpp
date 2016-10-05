﻿#ifdef _MSC_VER
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

//
////邻接顶点的余切权重计算
//void CotangentWeights(TriMs*TMesh, int vIndex, std::vector<double>&vweight, double &WeightSum, bool bNormalize)//计算一阶邻近点的各自cottan权重
//{
//	int NeighborNumber = TMesh->neighbors[vIndex].size();
//	vweight.resize(NeighborNumber);
//	WeightSum = 0;
//	std::vector<int>&NeiV = TMesh->neighbors[vIndex];
//	for (int i = 0; i < NeighborNumber; i++)
//	{
//		int j_nei = NeiV[i];
//		std::vector<int>tempnei;
//		Co_neighbor(TMesh, vIndex, j_nei, tempnei);
//		double cotsum = 0.0;
//		for (int j = 0; j < tempnei.size(); j++)
//		{
//			vec vivo = TMesh->vertices[vIndex] - TMesh->vertices[tempnei[j]];
//			vec vjvo = TMesh->vertices[j_nei] - TMesh->vertices[tempnei[j]];
//			double dotvector = vivo DOT vjvo;
//			dotvector = dotvector / sqrt(len2(vivo)*len2(vjvo) - dotvector*dotvector);
//			cotsum += dotvector;
//		}
//		vweight[i] = cotsum / 2.0;
//		WeightSum += vweight[i];
//	}
//
//	if (bNormalize)
//	{
//		for (int k = 0; k < NeighborNumber; ++k)
//		{
//			vweight[k] /= WeightSum;
//		}
//		WeightSum = 1.0;
//	}
//}
//
////获取两顶点的共同邻接顶点
//void Co_neighbor(TriMs *Tmesh, int u_id, int v_id, std::vector<int>&co_neiv)
//{
//	Tmesh->need_adjacentedges();
//	std::vector<int>&u_id_ae = Tmesh->adjancetedge[u_id];
//	int en = u_id_ae.size();
//	Tedge Co_Edge;
//	for (int i = 0; i < en; i++)
//	{
//		Tedge &ae = Tmesh->m_edges[u_id_ae[i]];
//		int opsi = ae.opposite_vertex(u_id);
//		if (opsi == v_id)
//		{
//			Co_Edge = ae;
//			break;
//		}
//	}
//	for (int i = 0; i < Co_Edge.m_adjacent_faces.size(); i++)
//	{
//		TriMs::Face af = Tmesh->faces[Co_Edge.m_adjacent_faces[i]];
//		for (int j = 0; j < 3; j++)
//		{
//			if ((af[j] != u_id) && (af[j] != v_id))
//			{
//				co_neiv.push_back(af[j]);
//			}
//		}
//	}
//}
////计算拉普拉斯矩阵
//void Get_Laplace_Matrix()
//{
//	int vn = m_BaseMesh->vertices.size();
//	int count0 = 0;
//	std::vector<int>begin_N(vn);
//	for (int i = 0; i < vn; i++)
//	{
//		begin_N[i] = count0;
//		count0 += m_BaseMesh->neighbors[i].size() + 1;
//	}
//	typedef Eigen::Triplet<double> T;
//	std::vector<T> tripletList(count0);
//	for (int i = 0; i < vn; i++)
//	{
//		VProperty & vi = m_vertices[i];
//		tripletList[begin_N[i]] = T(i, i, -vi.VSumWeight);
//		int nNbrs = vi.VNeighbors.size();
//		for (int k = 0; k < nNbrs; ++k)
//		{
//			tripletList[begin_N[i] + k + 1] = T(vi.VNeighbors[k], i, vi.VNeiWeight[k]);
//		}
//	}
//	m_Laplace_Matrix.resize(vn, vn);
//	m_Laplace_Matrix.setFromTriplets(tripletList.begin(), tripletList.end());
//
//}

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

	for (size_t i = 0; i != num; ++i) {
		out_triplet.emplace_back(Tpt_d(p1.idx(), ns[i].idx(), -nweigs[i] / s));
	}
	for (size_t i = 0; i != num; ++i) {
		x += nweigs[i] * ncords[i] / s;
	}
	//for (size_t i = 0; i != num; ++i) {
	//	out_triplet.emplace_back(Tpt_d(p1.idx(), ns[i].idx(), -1.0 / num));
	//}
	//	x = cv::Point2d(0.0, 0.0);
	//for (size_t i = 0; i != num; ++i) {
	//	x += ncords[i] / static_cast<double>(ncords.size());
	//}
	return cp1 - x;

}

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
	auto vertexsFin = std::vector<cv::Point2d>();
	auto segmentsFin = std::vector<cv::Point>();
	auto trisegsFin = std::vector<cv::Point>();

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

		const auto num = meshVertexs.size();
		//std::cout << "原始坐标" << std::endl;
		auto triplet = std::vector<Tpt_d>();
		auto laplace_cords = std::vector<cv::Point2d>();
		Eigen::VectorXd ori_x(num);
		Eigen::VectorXd ori_y(num);
		for(size_t i = 0;i != meshVertexs.size();++i) {
			laplace_cords.emplace_back(laplace_cord(mesh, meshVertexs[i], triplet));
			ori_x(i) = mesh.point(meshVertexs[i])[0];
			ori_y(i) = mesh.point(meshVertexs[i])[1];
		}

		//std::cout << ori_x << std::endl << ori_y << std::endl;

		triplet.emplace_back(Tpt_d(num + 0, 0, 1));
		triplet.emplace_back(Tpt_d(num + 1, 1, 1));
		triplet.emplace_back(Tpt_d(num + 2, 2, 1));
		
		auto laplace_mat = Spm_d(num + 3, meshVertexs.size());
		laplace_mat.setFromTriplets(triplet.begin(), triplet.end());

		//std::cout << std::endl << std::endl;

		//std::cout << "拉普拉斯坐标" << std::endl;
		auto delta = Eigen::MatrixX2d(num + 3, 2);

		for (size_t i = 0; i != laplace_cords.size(); ++i) {
			delta(i, 0) = laplace_cords[i].x;
			delta(i, 1) = laplace_cords[i].y;
		}

		delta(num + 0,0) = mesh.point(meshVertexs[0])[0];
		delta(num + 1,0) = mesh.point(meshVertexs[1])[0];
		delta(num + 2,0) = mesh.point(meshVertexs[2])[0];

		delta(num + 0,1) = mesh.point(meshVertexs[0])[1];
		delta(num + 1,1) = mesh.point(meshVertexs[1])[1];
		delta(num + 2,1) = mesh.point(meshVertexs[2])[1];

		//std::cout << delta_x << std::endl << delta_y << std::endl;
		laplace_mat.makeCompressed();
		Eigen::LeastSquaresConjugateGradient <Spm_d> lspg;
		lspg.setTolerance(0.0002);
		lspg.compute(laplace_mat);
		//if (lspg.info() != Eigen::Success) {
		//	//std::cout << "!!!" << std::endl;
		//}
		Eigen::MatrixX2d xx = lspg.solve(delta);

		//if (lspg.info() != Eigen::Success) {
		//	//std::cout << "!!!" << std::endl;
		//}
		//std::cout << "LeastSquaresConjugateGradient" << std::endl;
		//std::cout << xx << std::endl;
		//std::cout << yy << std::endl;

		//std::cout << "拉普拉斯矩阵 * LeastSquaresConjugateGradient" << std::endl;
		//std::cout << laplace_mat * xx << std::endl << std::endl;
		//std::cout << laplace_mat * yy << std::endl << std::endl;
		
		std::cout << "#iterations:     " << lspg.iterations() << std::endl;
		std::cout << "estimated error: " << lspg.error() << std::endl;

		for (auto p : vertexsFin) {
			draw_point(dst, p, cv::Scalar(0, 200, 0, 0), 5);
		}

		for (size_t i = 0; i != num;++i) {
			draw_point(dst, cv::Point(xx(i ,0), xx(i ,1)), cv::Scalar(0, 0, 255, 0), 2);
		}

		for (auto a : trisegsFin) {
			draw_line(dst, cv::Point(xx(a.x ,0), xx(a.x ,1)), cv::Point(xx(a.y, 0), xx(a.y, 1)));
		}
				
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
