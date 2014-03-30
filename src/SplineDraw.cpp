#include "UtilPreHead.hpp"
#include "SplineDraw.hpp"

MTRand SplineDraw::rng;
void SplineDraw::initConstants()
{
	akjRS.desc.MultisampleEnable=true;
	splineColor=0.0f;
	clicked=false;
	lastx=0.0f;
	lasty=0.0f;
	splineCounter=0.0;
	isSplineDrawing=false;
	indices =0;
	bufferIncrement=8192;
	segmentsPerLength=64;
	akjShader.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
	mappedVertices=0;
	className = L"SplineDraw";
	vertexSize=sizeof(SplineVertex);
	akjDSSread.desc.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
	akjDSSread.desc.StencilReadMask=0xFF;
	akjDSSread.desc.StencilWriteMask=0;
	akjDSS.desc.DepthEnable=false;
	akjDSS.desc.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO  ;
	akjDSS.desc.StencilEnable=false;
	akjDSS.desc.StencilReadMask=0xff;
	akjDSS.desc.StencilWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
	akjDSS.desc.FrontFace.StencilFunc=D3D11_COMPARISON_GREATER_EQUAL;
	akjDSS.desc.BackFace.StencilFunc=D3D11_COMPARISON_NEVER;
	akjDSS.desc.FrontFace.StencilFailOp=D3D11_STENCIL_OP_DECR_SAT;
	akjDSS.desc.FrontFace.StencilPassOp=D3D11_STENCIL_OP_INCR_SAT;
	akjDSS.desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_INCR;
	akjDSS.desc.DepthFunc=D3D11_COMPARISON_GREATER_EQUAL;
	akjVB.desc.ByteWidth=static_cast<UINT>(sizeof(SplineVertex)*segmentsPerLength*4);
	akjVB.mapType=D3D11_MAP_WRITE_NO_OVERWRITE;
	akjVB.desc.Usage=D3D11_USAGE_DYNAMIC;
	akjVB2.desc.Usage=D3D11_USAGE_DYNAMIC;
	akjIB.desc.ByteWidth=static_cast<UINT>(sizeof(short)*segmentsPerLength*4);
	akjVB2.desc.ByteWidth=static_cast<UINT>(sizeof(SpanVertex)*65536);
	akjVB2.mapType=D3D11_MAP_WRITE_DISCARD;
	akjVB2.setSlot(1);

	akjO.push_back(  &akjIB);
	akjO.push_back(  &akjVB2);
		colorMapSS.desc.AddressU=D3D11_TEXTURE_ADDRESS_MIRROR;
	colorMapSS.desc.Filter=D3D11_FILTER_ANISOTROPIC;
	colorMapSS.setSlot(0);
	colorMap.resObj.fileName=std::wstring(L"rainbow.cmap");
	colorMap.resObj.desc.Format=DXGI_FORMAT_R16G16B16A16_UNORM;
	colorMap.setSlot(0);
	colorMap.forPS=true;
	colorMap.forGS=true;
	colorMap.forVS=true;
	akjO.push_back(&colorMapSS);
	akjO.push_back(&colorMap);
	cleanedUp=false;
	akjShader.setShaders("VShader","SplineGeomShader","PShader");
	//akjVB.desc.ByteWidth=static_cast<UINT>(bufferIncrement*vertexSize);
	akjVB2.stride=sizeof(SpanVertex);
	akjIB.stride=sizeof(short);
	bufferVertices=segmentsPerLength*4;
	akjShader.parseFile( L"SplineShader.hlsl");
	shadeClone.setClone(&akjShader);
	shadeClone.setGShader("ControlPointGShader");
	shadeClone.setPShader("ControlPointPShader");
	akjO.push_back(&shadeClone);
	//akjRS.desc.FillMode=D3D11_FILL_WIREFRAME;
	akjBS.desc.RenderTarget[0].BlendEnable =TRUE;
	akjBS.desc.RenderTarget[0].SrcBlend =D3D11_BLEND_SRC_ALPHA;
	akjBS.desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	akjBS.desc.RenderTarget[0].BlendOp =D3D11_BLEND_OP_ADD;
	akjBS.desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	akjBS.desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	akjBS.desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
	akjBS.desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		akjBS.desc.RenderTarget[1].BlendEnable = TRUE;
	akjBS.desc.RenderTarget[1].SrcBlend =D3D11_BLEND_SRC_ALPHA;
	akjBS.desc.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	akjBS.desc.RenderTarget[1].BlendOp =D3D11_BLEND_OP_ADD;
	akjBS.desc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	akjBS.desc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	akjBS.desc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	akjBS.desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	akj::d3d::DrawLoopSlot dls;
	mouseFreeze=true;
		dls.eventType=akj::d3d::MBLD;
		dls.invPriority=16;
		dls.f.bind(this, &SplineDraw::sigStartDrawing);
	mySlots.push_back(dls);
		dls.eventType=akj::d3d::MBMD;
		dls.invPriority=13;
		dls.f.bind(this, &SplineDraw::sigStopDrawing);
mySlots.push_back(dls);
		dls.eventType=akj::d3d::MPOS;
		dls.invPriority=22;
		dls.f.bind(this, &SplineDraw::sigAddPoint);
	mySlots.push_back(dls);
		dls.eventType=akj::d3d::MWHEELV;
		dls.invPriority=15;
		dls.f.bind(this, &SplineDraw::sigZoom);
mySlots.push_back(dls);
		dls.eventType=akj::d3d::RESIZED;
		dls.invPriority=13;
		dls.f.bind(this, &SplineDraw::sigResized);
	mySlots.push_back(dls);
		dls.eventType=akj::d3d::SPLINESET;
		dls.invPriority=3;
		dls.f.bind(this, &SplineDraw::sigSplineSet);
mySlots.push_back(dls);
		dls.eventType=akj::d3d::WIREFRAME;
		dls.invPriority=3;
		dls.f.bind(this, &SplineDraw::sigSetWireFrame);
mySlots.push_back(dls);
		dls.eventType=akj::d3d::SPLINECOLORMAP;
		dls.invPriority=3;
		dls.f.bind(this, &SplineDraw::sigNewColorMap);
mySlots.push_back(dls);
	
			

		

		splineSet=&spareSet;
		totalSpans=0;
}
void SplineDraw::sigZoom(void* ptr)
{
	vec4f nowxy;
	nowxy(0)=((smooAI.x/viewPort.Width)*2.0f-1.0f);
	nowxy(1)=-((smooAI.y/viewPort.Height)*2.0f-1.0f);
	nowxy(3)=1.0f;
	zoomFactor= (*(float*)ptr)*0.04f;

	nowxy=inv(tMat)*nowxy;


	zoom2D(zoomFactor+1.0f,zoomFactor+1.0f, nowxy(0), nowxy(1));
	//translate2D(-akj::sgn(zoomFactor)*((smooAI.x/viewPort.Width)*2.0f-1.0f)*zoomFactor, akj::sgn(zoomFactor)*((smooAI.y/viewPort.Height)*2.0f-1.0f)*zoomFactor);
	zoomed=true;
}
void SplineDraw::sigStartDrawing(void* ptr)
{
	akjInput aI = *(akjInput*)ptr;
	if(isSplineDrawing){
		clicked=true;
		addPoint(aI, 1.0f,static_cast<float>(rng.rand()),true  );
		addPoint(aI, 1.0f,static_cast<float>(rng.rand()),false );
	}
	else{
		smooAI=aI;
		isSplineDrawing=true;
		tmp_cp.width=0.0f;
		tmp_cp.x=0.0f;
		tmp_cp.y=0.0f;
		std::set<akj::SplineSpanIndex> ssiSet=spareSet.startNew(tmp_cp);
		spareSet.commitUpdate(ssiSet);
		addPoint(aI, 0.0f,static_cast<float>(rng.rand()),false  );
		addPoint(aI, 1.0f,static_cast<float>(rng.rand()),true  );
	}

}
void SplineDraw::sigStopDrawing(void* ptr)
{
	isSplineDrawing=false;
	smooAI.addNew((akjInput*)ptr,0.9f);
}
void SplineDraw::sigSetWireFrame(void* ptr)
{

	akj::d3d::msgString* ms=(akj::d3d::msgString*)factoryGetMsg(akj::msg::SCREENMSG);
	
	ms->setColor(0.2f, 0.7f, 1.0f, 1.0f);

	
	if(akjRS.desc.FillMode==D3D11_FILL_WIREFRAME){
		ms->assign(L"Wireframe Mode OFF");
		akjRS.desc.FillMode=D3D11_FILL_SOLID;
	}
	else{
			ms->assign(L"Wireframe Mode ON");
		akjRS.desc.FillMode=D3D11_FILL_WIREFRAME;
	}
	akjRS.reCreate();
	PostMessage(ms->getMsgHandle(), akj::msg::SCREENMSG, NULL, (LPARAM) ms);
}
void SplineDraw::sigAddPoint(void* ptr)
{

	smooAI.addNew( (akjInput*)ptr,0.5f);
	akjInput aI=smooAI;
	if(isSplineDrawing){
		addPoint(aI, tmp_cp.width,tmp_cp.color, false);
	}
	else{
			lastx=(aI.x/viewPort.Width)*2.0f-1.0f;
			lasty=-((aI.y/viewPort.Height)*2.0f-1.0f);
	}

}
void SplineDraw::addPoint(akjInput& aI, float width, float color, bool perm)
{
		vec4f transcoord;
		transcoord(0)=(aI.x/viewPort.Width)*2.0f-1.0f;
		transcoord(1)=-((aI.y/viewPort.Height)*2.0f-1.0f);
		transcoord(2)=0.0f;
		transcoord(3)=1.0f;
		transcoord=inv(tMat)*transcoord;
		//(ext_timed-splineCounter)>2.5||
		if(perm){
			knotPos=10.0f;
			spareSet.commitUpdate(spareSet.adjustLastPoint(tmp_cp));
			tmp_cp.x=transcoord(0);
			tmp_cp.y=transcoord(1);
			tmp_cp.color=color;
			tmp_cp.width=width;
			spareSet.commitUpdate(spareSet.appendToCurrent(tmp_cp, knotPos));
			splineCounter=ext_timed;
			lastx=tmp_cp.x;
			lasty=tmp_cp.y;
			

		}
		else {
			tmp_cp.x=transcoord(0);
			tmp_cp.y=transcoord(1);
			tmp_cp.color=color;
			tmp_cp.width=width;
			akj::SplineCP dbltmp_cp=tmp_cp;
			dbltmp_cp.width=0.0;
			spareSet.commitUpdate(spareSet.adjustLastPoint(dbltmp_cp));
		}


}
void SplineDraw::sigMouseFreeze(void* ptr)
{
	mouseFreeze=false;
}
void SplineDraw::sigMouseUnFreeze(void* ptr)
{
	mouseFreeze=true;
}
int SplineDraw::initData(){
	zoomFactor=1.0f;
	zoomed=false;
	largestSpan=-1;
	largestSpline=-1;
	lastSpanIndx=0;
	isDataInit=true;
	float cap =   0.00f;
	unsigned int subSpans = static_cast<unsigned int>(std::floorf(1.0f*segmentsPerLength+0.5f)) ;
	float lengthPerVert = 1.0f/static_cast<float>(subSpans);

	SplineVertex sv;
	sv.t=-(cap+cap); sv.e=0.0f; sv.p=0.0f,sv.w=1.0f;
	vertVec.reserve(vertVec.size()+subSpans+4);
	indexVec.push_back(0);
	//vertVec.push_back(sv);
	sv.t=-lengthPerVert;
	vertVec.push_back(sv);
	//vertVec.push_back(sv);
	
	//vertVec.push_back(sv);
	//indexVec.push_back(0);
	for(unsigned short i=0; i<=subSpans;i++){
		sv.t=std::pow((i*lengthPerVert), 1.0f);
		vertVec.push_back(sv);
		indexVec.push_back(0);
	}
	mappedVertices=0;
	cap = 0.00f;
	sv.t+=lengthPerVert;
	vertVec.push_back(sv);
	//indexVec.push_back(0);
	sv.t+=cap;
	//	vertVec.push_back(sv);
	//vertVec.push_back(sv);
	
	//vertVec.push_back(sv);
	indexVec.push_back(0);
	vertsPerInstance=static_cast<int>(vertVec.size());
	vertices=static_cast<int>(vertVec.size());
	vertPtr=NULL;
	needVBUpdate=true;
	ied.clear();
	akjShader.inputElementPush(0,0,"TINFO");
	akjShader.inputElementPush(0,0,"CP", 1, D3D11_INPUT_PER_INSTANCE_DATA,1);
	akjShader.inputElementPush(16,1,"CP", 1, D3D11_INPUT_PER_INSTANCE_DATA,1);
	akjShader.inputElementPush(32,2,"CP", 1, D3D11_INPUT_PER_INSTANCE_DATA,1);
	akjShader.inputElementPush(48,3,"CP", 1, D3D11_INPUT_PER_INSTANCE_DATA,1);
	akjShader.inputElementPush(64,0,"DATA", 1, D3D11_INPUT_PER_INSTANCE_DATA,1);
	
	return 0;
}
void SplineDraw::updateData()
{
	//SplineSetStuff
	akj::SplineSpanIndex ssi; 
	needVBUpdate=false;
	bool carp=splineSet->getNextSpan(ssi);
	while(carp)
	{
		
		akj::SplineSpanRef snr=splineSet->getSpanRef(ssi.spline, ssi.span);
		float cap = (*snr.points)(0,2)  *0.04f*snr.span;
		unsigned int subSpans = static_cast<unsigned int>(std::floorf(snr.span*segmentsPerLength+0.5f)) ;
		float lengthPerVert = snr.span/static_cast<float>(subSpans);
		if(ssi.spline>largestSpline){
			largestSpline=ssi.spline;
			largestSpan=-1;
			lastSpanIndx=static_cast<int>(spanVec.size());
		}
		if(ssi.span>largestSpan){
			largestSpan=ssi.span;
			
			element_info.push_back(std::make_pair(static_cast<int>(vertices),static_cast<int>(vertVec.size())-static_cast<int>(vertices)));
			vertices=static_cast<unsigned int>(vertVec.size());
			arma::fmat cpm = (*snr.trnsMat)*(*snr.points);
			cpm.insert_cols(4, 1);
			cpm(0,4)=snr.span;
			spanVec.push_back( *((SpanVertex*)cpm.memptr())) ;
		}
		else{
			arma::fmat cpm = (*snr.trnsMat)*(*snr.points);
			cpm.insert_cols(4, 1);
			cpm(0,4)=snr.span;
			cpm(1,4)=snr.start;
			cpm(2,4)=static_cast<float>(snr.id);
			cpm(2,4)=4.0f/static_cast<float>(snr.ex);
			spanVec.at(lastSpanIndx+ssi.span)=*((SpanVertex*)cpm.memptr());
		}
		//per-span vertex
		carp=splineSet->getNextSpan(ssi);
		needVBUpdate=true;
		totalSpans++;
	}
	if(needVBUpdate){
	void * ptr= akjVB2.map();
		memcpy((char *)(ptr), (char *)(&spanVec[0]),sizeof(SpanVertex)*spanVec.size());
		akjVB2.unMap();
		needVBUpdate=false;
	}
	vertPtr=&vertVec[0];
	//tMat.eye();
	//tMat(0,3)=-1.0f;tMat(1,3)=1.0f;
	//tMat(0,0)=2.0f/(float)clientRect.right;
	//tMat(1,1)=-2.0f/(float)clientRect.bottom;
	if(mappedVertices<vertices) needVBUpdate=true;
}
void SplineDraw::drawAll(ID3D11DeviceContext* DC)
{

	updateData();

	if(vertices>bufferVertices){
		bufferVertices+=bufferIncrement;
		akjVB.desc.ByteWidth = bufferVertices*static_cast<UINT>(vertexSize);
		mappedVertices=0;
		akjVB.release();
		if(akjVB.create()!=S_OK) vertices=0;
	}
	if(DC) iDC=DC;
	else return;
	iDC->AddRef();
	float c[]={0.0f,0.0f,0.0f,0.0f};	
	ID3D11RenderTargetView* rtv;
	iDC->OMGetRenderTargets(1,&rtv,NULL);
	//iDC->ClearRenderTargetView( rtv, c);
	
	rtv->Release();
	if(needNewViewPort){
		unsigned int numVP=1;
		iDC->RSGetViewports(&numVP, &viewPort);
		needNewViewPort=false;
	}

	
	setState();
	 updateConstantBuffers();
	if(needVBUpdate){
			void * ptr= akjIB.map();
	memcpy((char *)(ptr), (char *)(&indexVec[0]), sizeof(short)*indexVec.size()  );
	akjIB.unMap();
		updateVertexBuffers();
	}
	//while(nextElement()>=0){
		isDrawing=true;
		needVBUpdate=false;

		if(!element_info.empty()){
			iDC->DrawInstanced(vertsPerInstance,static_cast<unsigned int>(element_info.size()),0,0);
			shadeClone.bind();
			iDC->DrawInstanced(vertsPerInstance,static_cast<unsigned int>(element_info.size()),0,0);
		}
		isDrawing=false;

	iDC->Release();
	iDC=NULL;
}

void SplineDraw::sigResized(void* ptr){
	clientRect= *(RECT*)ptr;
	needNewViewPort=true;
}

void SplineDraw::startLine()
{

}
void SplineDraw::endLine()
{

}

void SplineDraw::sigNewColorMap( void* ptr )
{
	colorMap.resObj.fileName = *(std::wstring*) ptr;
	colorMap.reCreate();
}
