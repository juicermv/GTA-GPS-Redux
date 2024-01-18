#include "GPS.h"

void GPS::UpdatePlayerPos()
{
    this->PlayerPos = FindPlayerCoors(0);
}

void GPS::calculatePath(
    CVector destPosn, 
    short& nodesCount, 
    CNodeAddress* resultNodes,
    float& gpsDistance
)
{
    destPosn.z = CWorld::FindGroundZForCoord(destPosn.x, destPosn.y);

    ThePaths.DoPathSearch
    (
        0,
        this->PlayerPos,
        CNodeAddress(),
        destPosn,
        resultNodes,
        &nodesCount,
        MAX_NODE_POINTS,
        &gpsDistance,
        999999.0f,
        NULL,
        999999.0f,
        (
            FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass != VEHICLE_BOAT
            && FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass != VEHICLE_BMX // Respect rules of traffic. (only if in valid vehicle & enabled in config)
            && cfg.RESPECT_LANE_DIRECTION
            ),
        CNodeAddress(),
        true,
        (FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT && cfg.ENABLE_WATER_GPS) // Whether to do water navigation
    );
}

CRGBA GPS::SetupColor(short color, bool friendly) {
    CRGBA clr;

    if (cfg.ENABLE_CUSTOM_CLRS)
    {
        switch (color)
        {
        case 0: // RED
            clr = cfg.CC_RED; break;
        case 1: // GREEN
            clr = cfg.CC_GREEN; break;
        case 2: // BLUE
            clr = cfg.CC_BLUE; break;
        case 3: // WHITE
            clr = cfg.CC_WHITE; break;
        case 4: // YELLOW
            clr = cfg.CC_YELLOW; break;
        case 5: // PURPLE
            clr = cfg.CC_PURPLE; break;
        case 6: // CYAN
            clr = cfg.CC_CYAN; break;
        case 7: // Depends on whether blip is friendly.
            if (friendly) {
                // BLUE
                clr = cfg.CC_BLUE;
            }
            else {
                // RED
                clr = cfg.CC_RED;
            }
            break;
        case 8: // DESTINATION
            clr = cfg.CC_YELLOW;
            break;
        }
    }
    else clr = CRadar::GetRadarTraceColour(color, 1, friendly);

    if (color < 1 || color > 8)
    {
        clr = CRGBA(cfg.GPS_LINE_R, cfg.GPS_LINE_G, cfg.GPS_LINE_B, cfg.GPS_LINE_A);
    }
    else {
        this->CurrentColor = clr;
    }

    return clr;
}

void GPS::Setup2dVertex(RwIm2DVertex& vertex, float x, float y, CRGBA clr) {
    vertex.x = x;
    vertex.y = y;
    vertex.u = vertex.v = 0.0f;
    vertex.z = CSprite2d::NearScreenZ + 0.0001f;
    vertex.rhw = CSprite2d::RecipNearClip;

    vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
}

void GPS::renderPath(
    CVector tracePos, 
    short color, 
    bool friendly, 
    short& nodesCount,
    CNodeAddress* resultNodes,  
    float& gpsDistance,
    RwIm2DVertex* lineVerts
)
{
    if (nodesCount <= 0) {
        return;
    }

    CVector2D nodePoints[MAX_NODE_POINTS];

    for (unsigned short i = 0; i < nodesCount; i++) {
        CPathNode* currentNode = ThePaths.GetPathNode(resultNodes[i]);
        CVector nodePosn = currentNode->GetNodeCoors();

        CVector2D tmpPoint;
        CRadar::TransformRealWorldPointToRadarSpace(tmpPoint, CVector2D(nodePosn.x, nodePosn.y));
        if (!FrontEndMenuManager.m_bDrawRadarOrMap)
            CRadar::TransformRadarPointToScreenSpace(nodePoints[i], tmpPoint);
        else {
            CRadar::LimitRadarPoint(tmpPoint);
            CRadar::TransformRadarPointToScreenSpace(nodePoints[i], tmpPoint);
            nodePoints[i].x *= static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
            nodePoints[i].y *= static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
            CRadar::LimitToMap(&nodePoints[i].x, &nodePoints[i].y);
        }
    }


    if 
    (
        !FrontEndMenuManager.m_bDrawRadarOrMap
        && reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST
    )
    {
        RECT rect;
        CVector2D posn;
        CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(-1.0f, -1.0f));
        rect.left = static_cast<LONG>(posn.x + 2.0f);
        rect.bottom = static_cast<LONG>(posn.y - 2.0f);
        CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(1.0f, 1.0f));
        rect.right = static_cast<LONG>(posn.x - 2.0f);
        rect.top = static_cast<LONG>(posn.y + 2.0f);
        GetD3DDevice<IDirect3DDevice9>()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        GetD3DDevice<IDirect3DDevice9>()->SetScissorRect(&rect);
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    
    CRGBA vColor = this->SetupColor(color, friendly);

    unsigned int vertIndex = 0;
    short lasti;
    CVector2D shift[2];
    for (unsigned short i = 0; i < (nodesCount - 1); i++) {
        vColor = this->SetupColor(color, friendly);
        
        // TODO: Move this (shift calculation) into a function.
        CVector2D dir = nodePoints[i + 1] - nodePoints[i]; // Direction between current node to next node
        float angle = atan2(dir.y, dir.x); // Convert direction to angle

        if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
            // 1.5707963 radians = 90 degrees
            shift[0].x = cosf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH;
            shift[0].y = sinf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH;
            shift[1].x = cosf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH;
            shift[1].y = sinf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH;
        }
        else {
            float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
            if (mp < 140.0f)
                mp = 140.0f;
            else if (mp > 960.0f)
                mp = 960.0f;
            mp = mp / 960.0f + 0.4f;
            shift[0].x = cosf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
            shift[0].y = sinf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
            shift[1].x = cosf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
            shift[1].y = sinf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
        }

        this->Setup2dVertex(                //
            lineVerts[vertIndex + 0],       //
            nodePoints[i].x + shift[0].x,   // CurrentNode*
            nodePoints[i].y + shift[0].y,   //
            vColor
        );

        this->Setup2dVertex(                //
            lineVerts[vertIndex + 1],       //
            nodePoints[i].x + shift[1].x,   // CurrentNode - CurrentNode*
            nodePoints[i].y + shift[1].y,   //
            vColor
        );

        this->Setup2dVertex(                    // NextNode*
            lineVerts[vertIndex + 2],           //    |
            nodePoints[i + 1].x + shift[0].x,   // CurrentNode - CurrentNode
            nodePoints[i + 1].y + shift[0].y,   //
            vColor
        );

        this->Setup2dVertex(
            lineVerts[vertIndex + 3],           // NextNode - NextNode*
            nodePoints[i + 1].x + shift[1].x,   //    |             |
            nodePoints[i + 1].y + shift[1].y,   // CurrentNode - CurrentNode
            vColor
        );
        
        lasti = i+1;
        vertIndex += 4;
    }
    
    // Create end segment
    CVector2D targetScreen;
    CVector2D tmpPoint;
    CRadar::TransformRealWorldPointToRadarSpace(tmpPoint, tracePos);
    if (!FrontEndMenuManager.m_bDrawRadarOrMap)
        CRadar::TransformRadarPointToScreenSpace(targetScreen, tmpPoint);
    else {
        CRadar::LimitRadarPoint(tmpPoint);
        CRadar::TransformRadarPointToScreenSpace(targetScreen, tmpPoint);
        targetScreen.x *= static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
        targetScreen.y *= static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
        CRadar::LimitToMap(&targetScreen.x, &targetScreen.y);
    }

    CVector2D dir = targetScreen - nodePoints[lasti]; // Direction between last node and the target position
    float angle = atan2(dir.y, dir.x); // Convert direction to angle

    if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
        // 1.5707963 radians = 90 degrees

        shift[0].x = cosf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH;
        shift[0].y = sinf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH;
        shift[1].x = cosf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH;
        shift[1].y = sinf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH;
    }
    else {
        float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
        if (mp < 140.0f)
            mp = 140.0f;
        else if (mp > 960.0f)
            mp = 960.0f;
        mp = mp / 960.0f + 0.4f;
        shift[0].x = cosf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
        shift[0].y = sinf(angle - 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
        shift[1].y = sinf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
        shift[1].x = cosf(angle + 1.5707963f) * cfg.GPS_LINE_WIDTH * mp;
    }

    //this->Log("DIR: " + std::to_string(dir.x) + ", " + std::to_string(dir.y));

    this->Setup2dVertex(
        lineVerts[vertIndex+0],
        nodePoints[lasti].x + shift[0].x,
        nodePoints[lasti].y + shift[0].y,
        vColor
    );

    this->Setup2dVertex(
        lineVerts[vertIndex+1],
        nodePoints[lasti].x + shift[1].x,
        nodePoints[lasti].y + shift[1].y,
        vColor
    );

    this->Setup2dVertex(
        lineVerts[vertIndex + 2],
        (nodePoints[lasti].x + (dir.x / 4.8)) + (shift[0].x / 2),
        (nodePoints[lasti].y + (dir.y / 4.8)) + (shift[0].y / 2),
        vColor
    );

    this->Setup2dVertex(
        lineVerts[vertIndex + 3],
        (nodePoints[lasti].x + (dir.x / 4.8)) + (shift[1].x / 2),
        (nodePoints[lasti].y + (dir.y / 4.8)) + (shift[1].y / 2),
        vColor
    );

    this->Setup2dVertex(
        lineVerts[vertIndex + 4],
        (nodePoints[lasti].x + (dir.x / 4.5)),
        (nodePoints[lasti].y + (dir.y / 4.5)),
        vColor
    );

    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, lineVerts, (4 * nodesCount)+1);

    if 
    (
        !FrontEndMenuManager.m_bDrawRadarOrMap
        && reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST
    )
    {
        GetD3DDevice<IDirect3DDevice9>()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    }

    gpsDistance += DistanceBetweenPoints(this->PlayerPos, ThePaths.GetPathNode(resultNodes[0])->GetNodeCoors());
}

// Check whether on BMX, will always return false if bmx support is enabled.
bool GPS::CheckBMX(CPed* player) {
    if (this->cfg.ENABLE_BMX)
        return false;
    
    return player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BMX;
}



void GPS::Run() {
    // Logging stuff
    if(cfg.LOGFILE_ENABLED)
        this->logfile.open("SA.GPS.LOG.txt", std::ios::out);

    // Load config values from file.
    Config::LoadConfig("SA.GPS.CONF.ini", this->cfg);

    // Parse custom colors

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

    plugin::Events::drawRadarOverlayEvent += [this]() { this->DrawRadarOverlayHandle(); };

    plugin::Events::drawRadarEvent += [this]() { this->DrawHudEventHandle(); };
}

#ifdef SAMP
LPVOID WINAPI init(LPVOID* lpParam) {
    MODULEINFO miSampDll;
    DWORD dwSampDllBaseAddr, dwSampDllEndAddr, dwCallAddr;

    GPSLine* sender = (GPSLine*)lpParam;

    stOpcodeRelCall* fnGameProc = (stOpcodeRelCall*)E_ADDR_GAMEPROCESS;

    // Check if E_ADDR_GAMEPROCESS opcode is a relative call (0xE8)
    while (fnGameProc->bOpcode != 0xE8)
        Sleep(100);

    while (true) {
        Sleep(100);

        // Get samp.dll module information to get base address and end address
        if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle("samp.dll"), &miSampDll, sizeof(MODULEINFO))) {
            continue;
        }

        // Some stupid calculation
        dwSampDllBaseAddr = (DWORD)miSampDll.lpBaseOfDll;
        dwSampDllEndAddr = dwSampDllBaseAddr + miSampDll.SizeOfImage;

        // Calculate destination address by offset and relative call opcode size
        dwCallAddr = fnGameProc->dwRelAddr + E_ADDR_GAMEPROCESS + 5;

        // Check if dwCallAddr is a samp.dll's hook address, 
        // to make sure this plugin hook (Events::gameProcessEvent) not replaced by samp.dll
        if (dwCallAddr >= dwSampDllBaseAddr && dwCallAddr <= dwSampDllEndAddr)
            break;
    }

    // Just wait a few secs for the game loaded fully to avoid any conflicts and crashes
    // I don't know what the elegant way is :)
    while (!FindPlayerPed(0))
        Sleep(5000);

    // Run the plugin
    sender->Run();

    // Reset the thread handle
    sender->hThread = NULL;

    return 0;
}

#endif

GPS::GPS() {
#ifdef SAMP
    this->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)init, (LPVOID)this, 0, (LPDWORD)NULL);
#else
    this->Run();
#endif
}



GPS::~GPS() {
    if(cfg.LOGFILE_ENABLED)
        this->logfile.close();

    #ifdef SAMP
        if (this->hThread != NULL)
            TerminateThread(this->hThread, 0);
    #endif
}

void GPS::renderMissionTrace(tRadarTrace trace) {
    //this->Log("Found mission objective blip.");

    switch (trace.m_nBlipType) {
    case 1:
        if (cfg.ENABLE_MOVING)
            destVec = CPools::GetVehicle(trace.m_nEntityHandle)->GetPosition();
        else
            renderMissionRoute = false; return;
        break;
    case 2:
        if (cfg.ENABLE_MOVING)
            destVec = CPools::GetPed(trace.m_nEntityHandle)->GetPosition();
        else
            renderMissionRoute = false; return;
        break;
    case 3:
        destVec = CPools::GetObject(trace.m_nEntityHandle)->GetPosition();
        break;
    case 6: // Searchlights
    case 8: // Airstripts
    case 0: // NONE???
        return;
    case 7: // Pickups
        //this->Log("Pickup detected. Not providing GPS navigation!");
        return;
    default:
        destVec = trace.m_vecPos;
        break;
    }

    //this->Log("DestVec: " + std::to_string(destVec.x) + ", " + std::to_string(destVec.y));
    this->calculatePath(destVec, missionNodesCount, m_ResultNodes, missionDistance);
    this->renderPath(destVec, trace.m_nColour, trace.m_bFriendly, missionNodesCount, m_ResultNodes, missionDistance, m_LineVerts);
}

void GPS::Log(std::string val) {
    if (!cfg.LOGFILE_ENABLED)
        return;

    if (this->logLines < 2048) {
        time_t timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char stime[128];
        strftime(stime, 128, "%c", localtime(&timenow));
        this->logfile << (std::string(stime) + " | " + val + "\n").c_str();
        this->logfile.flush();
        this->logLines++;
    }
    else {
        this->logfile.close();
        this->logfile.open("SA.GPS.LOG.txt", std::ios::out);
        this->logLines = 0;
        Log(val);
    }
}

const char* GPS::VectorToString(std::vector<tRadarTrace>& vec) {
    std::string out;
    for (unsigned short i = 0; i < (unsigned short)vec.size() - 1; i++) {
        out += std::to_string((int)vec.at(i).m_nRadarSprite) + ", " + std::to_string(DistanceBetweenPoints(this->PlayerPos, vec.at(i).m_vecPos)) + "\n\t";
    }
    const char* outChar = out.c_str();
    return outChar;
}

bool GPS::NavEnabled(CPed* player) {
    return (
            player
            && player->m_pVehicle
            && player->m_nPedFlags.bInVehicle
            && player->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE
            && player->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI
            && !CTheScripts::bMiniGameInProgress
            && !CheckBMX(player)
        );
}

void GPS::DrawRadarOverlayHandle() {
    CPed* playa = FindPlayerPed(0);

    if (!NavEnabled(playa))
        return;
    
    if (renderTargetRoute)
        this->renderPath(targetTracePos, -1, false, targetNodesCount, t_ResultNodes, targetDistance, t_LineVerts);

    if (renderMissionRoute)
    {
        try
        {
            this->renderMissionTrace(mTrace);
        }
        catch (const std::exception& e)
        {
            Log(e.what());
            renderMissionRoute = false;
        }
        
    }

    /*
    if (CTheScripts::IsPlayerOnAMission())
    {
        std::vector<tRadarTrace> traces;    // Couldn't use std::map due to some error in
        std::vector<float> trace_distances; // xstddef.

        this->Log("Looking for mission objective blip.");

        
        for (int i = 0; i < 174; i++)
        {

            tRadarTrace trace = CRadar::ms_RadarTrace[i];

            if
                (
                    trace.m_nRadarSprite == 0
                    && trace.m_nBlipDisplay > 1
                    && DistanceBetweenPoints(this->PlayerPos, trace.m_vecPos) > cfg.DISABLE_PROXIMITY
                    )
            {
                this->Log("Found contender.");
                traces.push_back(trace);

                if (trace.m_nColour == 8)
                    trace_distances.push_back(FLT_MAX); // Prioritize destination markers.
                else
                    trace_distances.push_back
                    (
                        DistanceBetweenPoints
                        (
                            this->PlayerPos,
                            trace.m_vecPos
                        )
                    );
            }

        }

        if (traces.size() > 0) {
            this->Log(this->VectorToString(traces));
            this->renderMissionTrace(
                traces
                [
                    std::distance
                    (
                        trace_distances.begin(),
                        std::max_element
                        (
                            trace_distances.begin(), trace_distances.end()
                        )
                    )
                ]
            );
        }
        
    }
    */
}


void GPS::GameEventHandle() {
    if (!NavEnabled(FindPlayerPed(0)))
        renderMissionRoute = false; return;

    this->UpdatePlayerPos();

    // Verify that blip still exists
    for (int i = 0; i < 175; i++) {
        if (CRadar::ms_RadarTrace[i].m_nEntityHandle != mTrace.m_nEntityHandle) {
            renderMissionRoute = false;
        }
        else
        {
            if (mTrace.m_nBlipDisplay > 1) {
                mTrace = CRadar::ms_RadarTrace[i];
                renderMissionRoute = true;
                break;
            }
        }
    }

    try {
        if (mTrace.m_nBlipDisplay <= 1 || DistanceBetweenPoints(this->PlayerPos, mTrace.m_vecPos) <= cfg.DISABLE_PROXIMITY)
        {
            renderMissionRoute = false;
        }
    }
    catch (const std::exception& e)
    {
        Log(e.what());
        renderMissionRoute = false;
    }

    if (FrontEndMenuManager.m_nTargetBlipIndex
        && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
        && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay
        && DistanceBetweenPoints(CVector2D(this->PlayerPos),
            CVector2D(CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos)) <= cfg.DISABLE_PROXIMITY
    )
    {
        CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
        FrontEndMenuManager.m_nTargetBlipIndex = 0;
        renderTargetRoute = false;
    }
    
    if (FrontEndMenuManager.m_nTargetBlipIndex
        && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
        && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay
        )
    {
        targetTracePos = CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos;
        this->calculatePath(targetTracePos, targetNodesCount, t_ResultNodes, targetDistance);
        renderTargetRoute = true;
    }
    
    
    if(FrontEndMenuManager.m_nTargetBlipIndex == 0) {
        renderTargetRoute = false;
    }

    try {
        if (CTheScripts::IsPlayerOnAMission()) {
            for (int i = 0; i < 175; i++) {
                tRadarTrace trace = CRadar::ms_RadarTrace[i];
                if (trace.m_nRadarSprite == 0
                    && trace.m_nBlipDisplay > 1
                    ) {
                    mTrace = trace;
                    renderMissionRoute = true;
                    break;
                }
            }
        }
        else {
            renderMissionRoute = false;
        }
    }
    catch (const std::exception& e)
    {
        Log(e.what());
        renderMissionRoute = false;
    }
}

std::string float2string(float in) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << in;
    return stream.str();
}

// Kilometers to Miles.
float KMtoM(float km) {
    return km / 1.609f;
}

// Meters to yards.
float mtoyard(float m) {
    return m * 1.094;
}

std::string makeDist(float dist, short units) {
    if (dist >= 1000.0f) {
        dist /= 1000.0f;
        return units == 0 ? float2string(dist) + "KM" :float2string(KMtoM(dist)) + "Mi";
    }

    return units == 0 ? float2string(dist) + "m" : float2string(mtoyard(dist)) + "yrd";
}

void GPS::DrawHudEventHandle() {
    if (!cfg.ENABLE_DISTANCE_TEXT)
        return;

    if (!NavEnabled(FindPlayerPed(0)))
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
            (char *)makeDist(
                DistanceBetweenPoints(
                    FindPlayerCoors(0), 
                    destVec
                ), 
                cfg.DISTANCE_UNITS
            ).c_str()
        );
    }

    if (renderTargetRoute) {

        CFont::SetOrientation(ALIGN_CENTER);
        CFont::SetColor(CRGBA(
            (unsigned char)cfg.GPS_LINE_R,
            (unsigned char)cfg.GPS_LINE_G,
            (unsigned char)cfg.GPS_LINE_B,
            (unsigned char)cfg.GPS_LINE_A
        ));

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
                DistanceBetweenPoints(
                    CVector2D(
                        this->PlayerPos
                    ),
                    CVector2D(
                        CRadar::ms_RadarTrace[
                            LOWORD(
                                FrontEndMenuManager.m_nTargetBlipIndex
                            )
                        ].m_vecPos
                    )
                ),
                cfg.DISTANCE_UNITS
            ).c_str()
        );
    }
}