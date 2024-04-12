#include "GPS.h"


void GPS::calculatePath(CVector destPosn,
                        short& nodesCount,
                        CNodeAddress* resultNodes,
                        float& gpsDistance) {
  ThePaths.DoPathSearch(
      0, player->GetPosition(), CNodeAddress(), destPosn, resultNodes, &nodesCount,
      MAX_NODE_POINTS, &gpsDistance, 999999.0f, NULL, 999999.0f,
      (player->m_pVehicle->m_nVehicleSubClass != VEHICLE_BOAT &&
       player->m_pVehicle->m_nVehicleSubClass !=
           VEHICLE_BMX  // Respect rules of traffic. (only if in valid vehicle &
                        // enabled in config)
       && cfg->RESPECT_LANE_DIRECTION),
      CNodeAddress(), false,
      (player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT &&
       cfg->ENABLE_WATER_GPS)  // Whether to do water navigation
  );
}

CRGBA GPS::SetupColor(short color, bool friendly) {
  CRGBA clr;

  if (cfg->ENABLE_CUSTOM_CLRS) {
    switch (color) {
      case 0:  // RED
        clr = cfg->CC_RED;
        break;
      case 1:  // GREEN
        clr = cfg->CC_GREEN;
        break;
      case 2:  // BLUE
        clr = cfg->CC_BLUE;
        break;
      case 3:  // WHITE
        clr = cfg->CC_WHITE;
        break;
      case 4:  // YELLOW
        clr = cfg->CC_YELLOW;
        break;
      case 5:  // PURPLE
        clr = cfg->CC_PURPLE;
        break;
      case 6:  // CYAN
        clr = cfg->CC_CYAN;
        break;
      case 7:  // Depends on whether blip is friendly.
        if (friendly) {
          // BLUE
          clr = cfg->CC_BLUE;
        } else {
          // RED
          clr = cfg->CC_RED;
        }
        break;
      case 8:  // DESTINATION
        clr = cfg->CC_YELLOW;
        break;
    }
  } else
    clr = CRadar::GetRadarTraceColour(color, 1, friendly);

  if (color < 1 || color > 8) {
    clr = cfg->GPS_LINE_CLR;
  } else {
    this->CurrentColor = clr;
  }

  return clr;
}

void GPS::Setup2dVertex(RwIm2DVertex& vertex, double x, double y, CRGBA clr) {
  vertex.x = x;
  vertex.y = y;
  vertex.u = vertex.v = 0.0f;
  vertex.z = CSprite2d::NearScreenZ + 0.0001f;
  vertex.rhw = CSprite2d::RecipNearClip;

  vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
}

void GPS::renderPath(CVector tracePos,
                     short color,
                     bool friendly,
                     short& nodesCount,
                     CNodeAddress* resultNodes,
                     float& gpsDistance,
                     RwIm2DVertex* lineVerts) {
  if (nodesCount <= 0) {
    return;
  }

  for (unsigned short i = 0; i < nodesCount; i++) {
    currentNode = ThePaths.GetPathNode(resultNodes[i]);
    nodePosn = currentNode->GetNodeCoors();

    CRadar::TransformRealWorldPointToRadarSpace(
        tmpPoint, CVector2D(nodePosn.x, nodePosn.y));
    if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
      CRadar::TransformRadarPointToScreenSpace(tmpNodePoints[i], tmpPoint);
    }
    else {
      CRadar::LimitRadarPoint(tmpPoint);
      CRadar::TransformRadarPointToScreenSpace(tmpNodePoints[i], tmpPoint);
      tmpNodePoints[i].x *= static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
      tmpNodePoints[i].y *= static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
      CRadar::LimitToMap(&tmpNodePoints[i].x, &tmpNodePoints[i].y);
    }
  }

  CRect scissorRect(0, 0, 0, 0);
  if (!FrontEndMenuManager.m_bDrawRadarOrMap &&
      reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps &
          D3DPRASTERCAPS_SCISSORTEST) {
    RECT rect;
    CVector2D posn;
    CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(-1.0f, -1.0f));
    rect.left = static_cast<LONG>(posn.x + 2.0f);
    rect.bottom = static_cast<LONG>(posn.y - 2.0f);
    CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(1.0f, 1.0f));
    rect.right = static_cast<LONG>(posn.x - 2.0f);
    rect.top = static_cast<LONG>(posn.y + 2.0f);
    GetD3DDevice<IDirect3DDevice9>()->SetRenderState(D3DRS_SCISSORTESTENABLE,
                                                     TRUE);
    GetD3DDevice<IDirect3DDevice9>()->SetScissorRect(&rect);

    scissorRect.Grow(rect.left, rect.right, rect.top, rect.bottom);
  }

  RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

  CRGBA vColor = this->SetupColor(color, friendly);

  // Create vertices for nodes
  vertIndex = 0;
  for (unsigned short i = 0; i < nodesCount - 1; i++) {
    vColor = this->SetupColor(color, friendly);

    dir = tmpNodePoints[i + 1] -
          tmpNodePoints[i];  // Direction between current node to next node
    angle = atan2(dir.y, dir.x);  // Convert direction to angle

    if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
      shift[0].x = cosf(angle - M_PI_2) * cfg->GPS_LINE_WIDTH;
      shift[0].y = sinf(angle - M_PI_2) * cfg->GPS_LINE_WIDTH;
      shift[1].x = cosf(angle + M_PI_2) * cfg->GPS_LINE_WIDTH;
      shift[1].y = sinf(angle + M_PI_2) * cfg->GPS_LINE_WIDTH;
    } else {
      float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
      if (mp < 140.0f)
        mp = 140.0f;
      else if (mp > 960.0f)
        mp = 960.0f;
      mp = mp / 960.0f + 0.4f;
      shift[0].x = cosf(angle - M_PI_2) * cfg->GPS_LINE_WIDTH * mp;
      shift[0].y = sinf(angle - M_PI_2) * cfg->GPS_LINE_WIDTH * mp;
      shift[1].x = cosf(angle + M_PI_2) * cfg->GPS_LINE_WIDTH * mp;
      shift[1].y = sinf(angle + M_PI_2) * cfg->GPS_LINE_WIDTH * mp;
    }

    // Only set up vertices for points visible on the radar. If the radar rect is 0 we assume the full screen map is open so we don't apply this optimization.
    if (scissorRect.IsPointInside(tmpNodePoints[i]) || (scissorRect.bottom + scissorRect.top + scissorRect.left + scissorRect.right) == 0) {
      this->Setup2dVertex(                  //
          lineVerts[vertIndex + 0],         //
          tmpNodePoints[i].x + shift[0].x,  // CurrentNode*
          tmpNodePoints[i].y + shift[0].y,  //
          vColor);

      this->Setup2dVertex(                  //
          lineVerts[vertIndex + 1],         //
          tmpNodePoints[i].x + shift[1].x,  // CurrentNode - CurrentNode*
          tmpNodePoints[i].y + shift[1].y,  //
          vColor);

      this->Setup2dVertex(                      // NextNode*
          lineVerts[vertIndex + 2],             //    |
          tmpNodePoints[i + 1].x + shift[0].x,  // CurrentNode - CurrentNode
          tmpNodePoints[i + 1].y + shift[0].y,  //
          vColor);

      this->Setup2dVertex(
          lineVerts[vertIndex + 3],             // NextNode - NextNode*
          tmpNodePoints[i + 1].x + shift[1].x,  //    |             |
          tmpNodePoints[i + 1].y + shift[1].y,  // CurrentNode - CurrentNode
          vColor);

      vertIndex += 4;
    }
  }

  RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, lineVerts, vertIndex);

  if (!FrontEndMenuManager.m_bDrawRadarOrMap &&
      reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps &
          D3DPRASTERCAPS_SCISSORTEST) {
    GetD3DDevice<IDirect3DDevice9>()->SetRenderState(D3DRS_SCISSORTESTENABLE,
                                                     FALSE);
  }

  gpsDistance += distCache.GetDist(
      player->GetPosition(), ThePaths.GetPathNode(resultNodes[0])->GetNodeCoors());
}

// Check whether on BMX, will always return false if bmx support is enabled.
constexpr bool GPS::CheckBMX(CPed* player) {
  if (this->cfg->ENABLE_BMX)
    return false;

  return player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BMX;
}

void GPS::Run() {
  cfg = new Config("SA.GPS.CONF.ini");

  if (cfg->LOGFILE_ENABLED)
    this->logfile.open("SA.GPS.LOG.txt", std::ios::out);

  for (short i = 0; i < 1024; i++) {
    pathNodesToStream[i] = 1;
  }

  for (unsigned short i = 0; i < 50000; i++) {
    pathNodes[i] = -1;
  }

  plugin::patch::SetPointer(0x44DE3C, &pathNodesToStream);
  plugin::patch::SetPointer(0x450D03, &pathNodesToStream);
  plugin::patch::SetPointer(0x451782, &pathNodes);
  plugin::patch::SetPointer(0x451904, &pathNodes);
  plugin::patch::SetPointer(0x451AC3, &pathNodes);
  plugin::patch::SetPointer(0x451B33, &pathNodes);
  plugin::patch::SetUInt(0x4518F8, 50000);
  plugin::patch::SetUInt(0x4519B0, 49950);

  plugin::Events::gameProcessEvent += [this]() { this->GameEventHandle(); };

  plugin::Events::drawRadarOverlayEvent +=
      [this]() { this->DrawRadarOverlayHandle(); };

  plugin::Events::drawRadarEvent += [this]() { this->DrawHudEventHandle(); };
}

GPS::GPS() {
  this->Run();
}

GPS::~GPS() {
  delete mTrace;
  delete cfg;
  delete player;

  if (cfg->LOGFILE_ENABLED)
    this->logfile.close();
}

void GPS::renderMissionTrace(tRadarTrace* trace) {
  // this->Log("Found mission objective blip.");
  if (!trace)
    return;

  if (trace->m_nBlipDisplay < 2) {
    renderMissionRoute = false;
    return;
  }

  switch (trace->m_nBlipType) {
    case 1:
      if (cfg->ENABLE_MOVING) {
        destVec = CPools::GetVehicle(trace->m_nEntityHandle)->GetPosition();
      } else {
        renderMissionRoute = false;
        return;
      }
      break;
    case 2:
      if (cfg->ENABLE_MOVING) {
        destVec = CPools::GetPed(trace->m_nEntityHandle)->GetPosition();
      } else {
        renderMissionRoute = false;
        return;
      }
      break;
    case 3:
      destVec = CPools::GetObject(trace->m_nEntityHandle)->GetPosition();
      break;
    case 6:  // Searchlights
    case 8:  // Airstripts
    case 0:  // NONE???
      return;
    case 7:  // Pickups
      // this->Log("Pickup detected. Not providing GPS navigation!");
      return;
    default:
      destVec = trace->m_vecPos;
      break;
  }

  // this->Log("DestVec: " + std::to_string(destVec.x) + ", " +
  // std::to_string(destVec.y));
  this->calculatePath(destVec, missionNodesCount, m_ResultNodes,
                      missionDistance);
  this->renderPath(destVec, trace->m_nColour, trace->m_bFriendly,
                   missionNodesCount, m_ResultNodes, missionDistance,
                   m_LineVerts);
}

void GPS::Log(std::string val) {
  if (!cfg->LOGFILE_ENABLED)
    return;

  if (this->logLines < 2048) {
    time_t timenow =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char stime[128];
    strftime(stime, 128, "%c", localtime(&timenow));
    this->logfile << (std::string(stime) + " | " + val + "\n").c_str();
    this->logfile.flush();
    this->logLines++;
  } else {
    this->logfile.close();
    this->logfile.open("SA.GPS.LOG.txt", std::ios::out);
    this->logLines = 0;
    Log(val);
  }
}

const char* GPS::VectorToString(std::vector<tRadarTrace>& vec) {
  std::string out;
  for (unsigned short i = 0; i < (unsigned short)vec.size() - 1; i++) {
    out += std::to_string((int)vec.at(i).m_nRadarSprite) + ", " +
           std::to_string(
               distCache.GetDist(player->GetPosition(), vec.at(i).m_vecPos)) +
           "\n\t";
  }
  const char* outChar = out.c_str();
  return outChar;
}

constexpr bool GPS::NavEnabled(CPed* player) {
  return (player && player->m_pVehicle && player->m_nPedFlags.bInVehicle &&
          player->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE &&
          player->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI &&
          !CTheScripts::bMiniGameInProgress && !CheckBMX(player));
}

void GPS::DrawRadarOverlayHandle() {
  if (!NavEnabled(player))
    return;

  if (renderTargetRoute)
    this->renderPath(targetTracePos, -1, false, targetNodesCount, t_ResultNodes,
                     targetDistance, t_LineVerts);

  if (renderMissionRoute) {
    try {
      this->renderMissionTrace(mTrace);
    } catch (const std::exception& e) {
      Log(e.what());
      renderMissionRoute = false;
    }
  }
}

void GPS::GameEventHandle() {
  player = FindPlayerPed(0);

  renderTargetRoute =
      FrontEndMenuManager.m_nTargetBlipIndex == 0 ? false : true;

  if (!mTrace)
    renderMissionRoute = false;
  else {
    renderMissionRoute = mTrace->m_bInUse;
    if (mTrace->m_nBlipDisplay < 2 ||
        distCache.GetDist(player->GetPosition(), mTrace->m_vecPos) <=
            cfg->DISABLE_PROXIMITY) {
      renderMissionRoute = false;
    }

    if (!renderMissionRoute)
      mTrace = nullptr;
  }

  if (!NavEnabled(player)) {
    renderMissionRoute = false;
    return;
  }

  if (FrontEndMenuManager.m_nTargetBlipIndex &&
      CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)]
              .m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex) &&
      CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)]
          .m_nBlipDisplay &&
      distCache.GetDist(
          player->GetPosition(),
              CRadar::ms_RadarTrace[LOWORD(
                                        FrontEndMenuManager.m_nTargetBlipIndex)]
                  .m_vecPos) <= cfg->DISABLE_PROXIMITY) {
    CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
    FrontEndMenuManager.m_nTargetBlipIndex = 0;
    renderTargetRoute = false;
  }

  if (FrontEndMenuManager.m_nTargetBlipIndex &&
      CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)]
              .m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex) &&
      CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)]
          .m_nBlipDisplay) {
    targetTracePos =
        CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)]
            .m_vecPos;
    this->calculatePath(targetTracePos, targetNodesCount, t_ResultNodes,
                        targetDistance);
    renderTargetRoute = true;
  }

  try {
    if (CTheScripts::IsPlayerOnAMission()) {
      for (int i = 0; i < 175; i++) {
        tRadarTrace* trace = &CRadar::ms_RadarTrace[i];
        if (trace) {
          if (trace->m_nRadarSprite == 0 && trace->m_nBlipDisplay > 1) {
            mTrace = trace;
            renderMissionRoute = true;
            break;
          }
        }
      }
    } else {
      renderMissionRoute = false;
    }
  } catch (const std::exception& e) {
    Log(e.what());
    renderMissionRoute = false;
  }
}

// Meters to yards.
constexpr float mtoyard(float m) {
  return m * 1.094f;
}

std::string Float2String(float in, unsigned char precision = 2) {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << in;
  return stream.str();
}

std::string makeDist(float dist, bool units) {
  // 1 Unit of distance = 1 meter.
  switch (units) {
    case 0:
      if (dist > 999) {
        return Float2String(dist / 1000, 1) + " KM";
      } else {
        return Float2String(dist, 0) + " m";
      }
      break;
    case 1:
      dist = mtoyard(dist);
      if (dist > 599) {
        return Float2String(dist / 1760, 1) + " Mi";
      } else {
        return Float2String(dist, 0) + " yrds";
      }
      break;
  }
}

void GPS::DrawHudEventHandle() {
  if (!cfg->ENABLE_DISTANCE_TEXT)
    return;

  if (!NavEnabled(player))
    return;

  if (renderMissionRoute) {
    CFont::SetOrientation(ALIGN_CENTER);
    CFont::SetColor(this->CurrentColor);
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
        point.x,
        point.y + 8.0f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f,
        (char*)makeDist(distCache.GetDist(FindPlayerCoors(0), destVec),
                        cfg->DISTANCE_UNITS)
            .c_str());
  }

  if (renderTargetRoute) {
    CFont::SetOrientation(ALIGN_CENTER);
    CFont::SetColor(cfg->GPS_LINE_CLR);

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
        point.x,
        point.y - 20.0f * static_cast<float>(RsGlobal.maximumHeight) / 448.0f,
        (char*)makeDist(
            distCache.GetDist(
                CVector(player->GetPosition()),
                CVector(CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager
                                                           .m_nTargetBlipIndex)]
                              .m_vecPos)),
            cfg->DISTANCE_UNITS)
            .c_str());
  }
}