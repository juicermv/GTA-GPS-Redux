#include "GPS.h"

void GPS::Run()
{
	logger = util::Logger(cfg.LOGFILE_ENABLED);

        std::fill(std::begin(pathNodesToStream), std::end(pathNodesToStream), 1);
        std::fill(std::begin(pathNodes), std::end(pathNodes), -1);

        plugin::patch::SetPointer(0x44DE3C, pathNodesToStream.data());
        plugin::patch::SetPointer(0x450D03, pathNodesToStream.data());
        plugin::patch::SetPointer(0x451782, pathNodes.data());
        plugin::patch::SetPointer(0x451904, pathNodes.data());
        plugin::patch::SetPointer(0x451AC3, pathNodes.data());
        plugin::patch::SetPointer(0x451B33, pathNodes.data());
	plugin::patch::SetUInt(0x4518F8, 50000);
	plugin::patch::SetUInt(0x4519B0, 49950);

	plugin::Events::gameProcessEvent += [this]()
	{ this->GameEventHandle(); };

	plugin::Events::drawRadarOverlayEvent += [this]()
	{ this->DrawRadarOverlayHandle(); };

	plugin::Events::drawRadarEvent += [this]()
	{ this->DrawHudEventHandle(); };
}

void GPS::calculatePath(CVector destPosn, short &nodesCount, CNodeAddress *resultNodes, float &gpsDistance)
{
	ThePaths.DoPathSearch(
		0, player->GetPosition(), CNodeAddress(), destPosn, resultNodes, &nodesCount, MAX_NODE_POINTS, &gpsDistance,
		999999.0f, NULL, 999999.0f,
		(player->m_pVehicle->m_nVehicleSubClass != VEHICLE_BOAT &&
		 player->m_pVehicle->m_nVehicleSubClass != VEHICLE_BMX // Respect rules of traffic. (only if in valid
															   // vehicle & enabled in config)
		 && cfg.RESPECT_LANE_DIRECTION),
		CNodeAddress(), false,
		(player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT &&
		 cfg.ENABLE_WATER_GPS) // Whether to do water navigation
	);
}

// Events

constexpr void GPS::DrawRadarOverlayHandle()
{
	if (!util::NavEnabled(this->cfg, player))
		return;

	if (renderTargetRoute)
                this->renderPath(targetTracePos, -1, false, targetNodesCount, t_ResultNodes.data(), targetDistance, t_LineVerts.data());

	if (renderMissionRoute)
	{
		try
		{
			this->renderMissionTrace(mTrace);
		}
		catch (const std::exception &e)
		{
			logger.Log(e.what());
			renderMissionRoute = false;
		}
	}
}

void GPS::GameEventHandle()
{
	player = FindPlayerPed(0);

	renderTargetRoute = FrontEndMenuManager.m_nTargetBlipIndex == 0 ? false : true;

	if (!mTrace)
		renderMissionRoute = false;
	else
	{
		renderMissionRoute = mTrace->m_bInUse;
		if (mTrace->m_nBlipDisplay < 2 ||
			this->distCache.GetDist2D(player->GetPosition(), mTrace->m_vecPos) <= cfg.DISABLE_PROXIMITY)
		{
			renderMissionRoute = false;
		}

		if (!renderMissionRoute)
			mTrace = nullptr;
	}

	if (!util::NavEnabled(this->cfg, player))
	{
		renderMissionRoute = false;
		return;
	}

	if (FrontEndMenuManager.m_nTargetBlipIndex &&
		CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter ==
			HIWORD(FrontEndMenuManager.m_nTargetBlipIndex) &&
		CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay &&
		this->distCache.GetDist2D(player->GetPosition(),
								  CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos) <=
			cfg.DISABLE_PROXIMITY)
	{
		CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
		FrontEndMenuManager.m_nTargetBlipIndex = 0;
		renderTargetRoute = false;
	}

	if (FrontEndMenuManager.m_nTargetBlipIndex &&
		CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter ==
			HIWORD(FrontEndMenuManager.m_nTargetBlipIndex) &&
		CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay)
	{
		targetTracePos = CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos;
                this->calculatePath(targetTracePos, targetNodesCount, t_ResultNodes.data(), targetDistance);
		renderTargetRoute = true;
	}

	try
	{
		if (CTheScripts::IsPlayerOnAMission())
		{
			for (int i = 0; i < 175; i++)
			{
				tRadarTrace *trace = &CRadar::ms_RadarTrace[i];
				if (trace)
				{
					if (trace->m_nRadarSprite == 0 && trace->m_nBlipDisplay > 1)
					{
						mTrace = trace;
						renderMissionRoute = true;
						break;
					}
				}
			}
		}
		else
		{
			renderMissionRoute = false;
		}
	}
	catch (const std::exception &e)
	{
		logger.Log(e.what());
		renderMissionRoute = false;
	}
}

constexpr void GPS::DrawHudEventHandle()
{
	if (!cfg.ENABLE_DISTANCE_TEXT)
		return;

	if (!NavEnabled(this->cfg, player))
		return;

	if (renderMissionRoute)
	{
		CFont::SetOrientation(ALIGN_CENTER);
		CFont::SetColor(SetupColor(this->mTrace->m_nColour, this->mTrace->m_bFriendly, cfg));
		CFont::SetBackground(false, false);
		CFont::SetWrapx(500.0f);
		CFont::SetScale(0.3f * static_cast<float>(RsGlobal.maximumWidth) / 640.0f,
						0.6f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f);
		CFont::SetFontStyle(FONT_SUBTITLES);
		CFont::SetProportional(true);
		CFont::SetDropShadowPosition(1);
		CFont::SetDropColor(CRGBA(0, 0, 0, 180));

		CVector2D point;
		CRadar::TransformRadarPointToScreenSpace(point, CVector2D(0.0f, -1.0f));
		CFont::PrintString(
			point.x, point.y + 8.0f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f,
			(char *)util::makeDist(this->distCache.GetDist(FindPlayerCoors(0), destVec), cfg.DISTANCE_UNITS).c_str());
	}

	if (renderTargetRoute)
	{
		CFont::SetOrientation(ALIGN_CENTER);
		CFont::SetColor(cfg.GPS_LINE_CLR);

		CFont::SetBackground(false, false);
		CFont::SetWrapx(500.0f);
		CFont::SetScale(0.3f * static_cast<float>(RsGlobal.maximumWidth) / 640.0f,
						0.6f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f);
		CFont::SetFontStyle(FONT_SUBTITLES);
		CFont::SetProportional(true);
		CFont::SetDropShadowPosition(1);
		CFont::SetDropColor(CRGBA(0, 0, 0, 180));

		CVector2D point;
		CRadar::TransformRadarPointToScreenSpace(point, CVector2D(0.0f, 1.0f));
		CFont::PrintString(
			point.x, point.y - 20.0f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f,
			(char *)util::makeDist(
				this->distCache.GetDist(
					CVector(player->GetPosition()),
					CVector(CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos)),
				cfg.DISTANCE_UNITS)
				.c_str());
	}
}

// Rendering

void GPS::renderPath(CVector tracePos, short color, bool friendly, short &nodesCount, CNodeAddress *resultNodes,
					 float &gpsDistance, RwIm2DVertex *lineVerts)
{
	if (nodesCount <= 0)
	{
		return;
	}

	for (unsigned short i = 0; i < nodesCount; i++)
	{
		currentNode = ThePaths.GetPathNode(resultNodes[i]);
		nodePosn = currentNode->GetNodeCoors();

		CRadar::TransformRealWorldPointToRadarSpace(tmpPoint, CVector2D(nodePosn.x, nodePosn.y));
		if (!FrontEndMenuManager.m_bDrawRadarOrMap)
		{
			CRadar::TransformRadarPointToScreenSpace(tmpNodePoints[i], tmpPoint);
		}
		else
		{
			CRadar::LimitRadarPoint(tmpPoint);
			CRadar::TransformRadarPointToScreenSpace(tmpNodePoints[i], tmpPoint);
			tmpNodePoints[i].x *= static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
			tmpNodePoints[i].y *= static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
			CRadar::LimitToMap(&tmpNodePoints[i].x, &tmpNodePoints[i].y);
		}
	}

	CRect scissorRect(0, 0, 0, 0);
	if (!FrontEndMenuManager.m_bDrawRadarOrMap &&
		reinterpret_cast<D3DCAPS9 const *>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
	{
		RECT rect;
		CVector2D posn;
		CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(-1.0f, -1.0f));
		rect.left = static_cast<LONG>(posn.x + 2.0f);
		rect.bottom = static_cast<LONG>(posn.y - 2.0f);
		CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(1.0f, 1.0f));
		rect.right = static_cast<LONG>(posn.x - 2.0f);
		rect.top = static_cast<LONG>(posn.y + 2.0f);
		reinterpret_cast<IDirect3DDevice9 *>(GetD3DDevice())->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		reinterpret_cast<IDirect3DDevice9 *>(GetD3DDevice())->SetScissorRect(&rect);

		scissorRect.Grow(rect.left, rect.right, rect.top, rect.bottom);
	}

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

	CRGBA vColor = SetupColor(color, friendly, cfg);

	// Create vertices for nodes
	vertIndex = 0;
	for (unsigned short i = 0; i < nodesCount - 1; i++)
	{
		vColor = SetupColor(color, friendly, cfg);

		dir = tmpNodePoints[i + 1] - tmpNodePoints[i]; // Direction between current node to next
													   // node
		angle = atan2(dir.y, dir.x);				   // Convert direction to angle

		if (!FrontEndMenuManager.m_bDrawRadarOrMap)
		{
			shift[0].x = cosf(angle - M_PI_2) * cfg.GPS_LINE_WIDTH;
			shift[0].y = sinf(angle - M_PI_2) * cfg.GPS_LINE_WIDTH;
			shift[1].x = cosf(angle + M_PI_2) * cfg.GPS_LINE_WIDTH;
			shift[1].y = sinf(angle + M_PI_2) * cfg.GPS_LINE_WIDTH;
		}
		else
		{
			float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
			if (mp < 140.0f)
				mp = 140.0f;
			else if (mp > 960.0f)
				mp = 960.0f;
			mp = mp / 960.0f + 0.4f;
			shift[0].x = cosf(angle - M_PI_2) * cfg.GPS_LINE_WIDTH * mp;
			shift[0].y = sinf(angle - M_PI_2) * cfg.GPS_LINE_WIDTH * mp;
			shift[1].x = cosf(angle + M_PI_2) * cfg.GPS_LINE_WIDTH * mp;
			shift[1].y = sinf(angle + M_PI_2) * cfg.GPS_LINE_WIDTH * mp;
		}

		// Only set up vertices for points visible on the radar. If the
		// radar rect is 0 we assume the full screen map is open so we
		// don't apply this optimization.
		if (scissorRect.IsPointInside(tmpNodePoints[i]) ||
			(scissorRect.bottom + scissorRect.top + scissorRect.left + scissorRect.right) == 0)
		{
			util::Setup2dVertex(				 //
				lineVerts[vertIndex + 0],		 //
				tmpNodePoints[i].x + shift[0].x, // CurrentNode*
				tmpNodePoints[i].y + shift[0].y, //
				vColor);

			util::Setup2dVertex(				 //
				lineVerts[vertIndex + 1],		 //
				tmpNodePoints[i].x + shift[1].x, // CurrentNode - CurrentNode*
				tmpNodePoints[i].y + shift[1].y, //
				vColor);

			util::Setup2dVertex(					 // NextNode*
				lineVerts[vertIndex + 2],			 //    |
				tmpNodePoints[i + 1].x + shift[0].x, // CurrentNode - CurrentNode
				tmpNodePoints[i + 1].y + shift[0].y, //
				vColor);

			util::Setup2dVertex(lineVerts[vertIndex + 3],			 // NextNode - NextNode*
								tmpNodePoints[i + 1].x + shift[1].x, //    | |
								tmpNodePoints[i + 1].y + shift[1].y, // CurrentNode - CurrentNode
								vColor);

			vertIndex += 4;
		}
	}

	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, lineVerts, vertIndex);

	if (!FrontEndMenuManager.m_bDrawRadarOrMap &&
		reinterpret_cast<D3DCAPS9 const *>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
	{
		reinterpret_cast<IDirect3DDevice9 *>(GetD3DDevice())->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}

	gpsDistance += this->distCache.GetDist(player->GetPosition(), ThePaths.GetPathNode(resultNodes[0])->GetNodeCoors());
}

constexpr void GPS::renderMissionTrace(tRadarTrace *trace)
{
	// this->Log("Found mission objective blip.");
	if (!trace)
		return;

	if (trace->m_nBlipDisplay < 2)
	{
		renderMissionRoute = false;
		return;
	}

	switch (trace->m_nBlipType)
	{
	case 1:
		if (cfg.ENABLE_MOVING)
		{
			destVec = CPools::GetVehicle(trace->m_nEntityHandle)->GetPosition();
		}
		else
		{
			renderMissionRoute = false;
			return;
		}
		break;
	case 2:
		if (cfg.ENABLE_MOVING)
		{
			destVec = CPools::GetPed(trace->m_nEntityHandle)->GetPosition();
		}
		else
		{
			renderMissionRoute = false;
			return;
		}
		break;
	case 3:
		destVec = CPools::GetObject(trace->m_nEntityHandle)->GetPosition();
		break;
	case 6: // Searchlights
	case 8: // Airstripts
	case 0: // NONE???
		return;
	case 7: // Pickups
		renderMissionRoute = false;
		logger.Log("Pickup detected. Not providing GPS navigation!");
		return;
	default:
		destVec = trace->m_vecPos;
		break;
	}

	// this->Log("DestVec: " + std::to_string(destVec.x) + ", " +
	// std::to_string(destVec.y));
	if (renderMissionRoute)
	{
                this->calculatePath(destVec, missionNodesCount, m_ResultNodes.data(), missionDistance);

                this->renderPath(destVec, trace->m_nColour, trace->m_bFriendly, missionNodesCount, m_ResultNodes.data(),
                                                 missionDistance, m_LineVerts.data());
	}
}
