#include "GPSLine.h"

void GPSLine::UpdatePlayerPos()
{
    this->PlayerPos = FindPlayerCoors(0);
}

void GPSLine::calculatePath(
    CVector destPosn, 
    short& nodesCount, 
    CNodeAddress* resultNodes,
    CVector2D* nodePoints, 
    float* nodeHeights,
    float& gpsDistance
)
{
    destPosn.z = CWorld::FindGroundZForCoord(destPosn.x, destPosn.y);

    if (DistanceBetweenPoints(PrevPos, this->PlayerPos) >= 20.0f || (this->PrevDest.Magnitude() - destPosn.Magnitude()) != 0) {
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
            (FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass != VEHICLE_BOAT), // Respect rules of traffic. (only if in valid vehicle)
            CNodeAddress(),
            true,
            (FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT && ENABLE_WATER_GPS) // Whether to do water navigation
        );
        nodePoints[0] = CVector2D(this->PlayerPos);
        this->PrevPos = this->PlayerPos;
        this->PrevDest = destPosn;
    }

    for (unsigned short i = 0; i < nodesCount; i++) {
        // TODO you can extract width from this as well
        CPathNode *currentNode = ThePaths.GetPathNode(resultNodes[i]);
        CVector nodePosn = currentNode->GetNodeCoors();


        nodeHeights[i] = 20 / CWorld::FindGroundZForCoord(nodePosn.x, nodePosn.y);

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
}

CRGBA GPSLine::SetupColor(short color, bool friendly, float height) {
    CRGBA clr;

    if (ENABLE_CUSTOM_CLRS)
    {
        switch (color)
        {
        case 0: // RED
            clr = CC_RED; break;
        case 1: // GREEN
            clr = CC_GREEN; break;
        case 2: // BLUE
            clr = CC_BLUE; break;
        case 3: // WHITE
            clr = CC_WHITE; break;
        case 4: // YELLOW
            clr = CC_YELLOW; break;
        case 5: // PURPLE
            clr = CC_PURPLE; break;
        case 6: // CYAN
            clr = CC_CYAN; break;
        case 7: // Depends on whether blip is friendly.
            if (friendly) {
                // BLUE
                clr = CC_BLUE;
            }
            else {
                // RED
                clr = CC_RED;
            }
            break;
        case 8: // DESTINATION
            clr = CC_YELLOW;
            break;
        }
    }
    else clr = CRadar::GetRadarTraceColour(color, 1, friendly);

    if (color < 1 || color > 8)
    {
        clr = CRGBA(GPS_LINE_R, GPS_LINE_G, GPS_LINE_B, GPS_LINE_A);
    }

    clr.r = std::clamp(clr.r + height, 0.0f, 255.0f);
    clr.g = std::clamp(clr.g + height, 0.0f, 255.0f);
    clr.b = std::clamp(clr.b + height, 0.0f, 255.0f);

    return clr;
}

void GPSLine::Setup2dVertex(RwIm2DVertex& vertex, float x, float y, CRGBA clr) {
    vertex.x = x;
    vertex.y = y;
    vertex.u = vertex.v = 0.0f;
    vertex.z = CSprite2d::NearScreenZ + 0.0001f;
    vertex.rhw = CSprite2d::RecipNearClip;

    vertex.emissiveColor = RWRGBALONG(clr.r, clr.g, clr.b, clr.a);
}

void GPSLine::renderPath(
    CVector tracePos, 
    short color, 
    bool friendly, 
    short& nodesCount, 
    bool& gpsShown, 
    CNodeAddress* resultNodes, 
    CVector2D* nodePoints, 
    float* nodeHeights,
    float& gpsDistance,
    RwIm2DVertex* lineVerts
)
{
    if (nodesCount <= 0) {
        return;
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
        GetD3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        GetD3DDevice()->SetScissorRect(&rect);
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    
    CRGBA vColor = this->SetupColor(color, friendly, false);

    unsigned int vertIndex = 0;
    short lasti;
    CVector2D shift[2];
    for (unsigned short i = 0; i < (nodesCount - 1); i++) {
        vColor = this->SetupColor(color, friendly, nodeHeights[i]);
        
        // TODO: Move this (shift calculation) into a function.
        CVector2D dir = nodePoints[i + 1] - nodePoints[i]; // Direction between current node to next node
        float angle = atan2(dir.y, dir.x); // Convert direction to angle

        if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
            // 1.5707963 radians = 90 degrees
            shift[0].x = cosf(angle - 1.5707963f) * GPS_LINE_WIDTH;
            shift[0].y = sinf(angle - 1.5707963f) * GPS_LINE_WIDTH;
            shift[1].x = cosf(angle + 1.5707963f) * GPS_LINE_WIDTH;
            shift[1].y = sinf(angle + 1.5707963f) * GPS_LINE_WIDTH;
        }
        else {
            float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
            if (mp < 140.0f)
                mp = 140.0f;
            else if (mp > 960.0f)
                mp = 960.0f;
            mp = mp / 960.0f + 0.4f;
            shift[0].x = cosf(angle - 1.5707963f) * GPS_LINE_WIDTH * mp;
            shift[0].y = sinf(angle - 1.5707963f) * GPS_LINE_WIDTH * mp;
            shift[1].x = cosf(angle + 1.5707963f) * GPS_LINE_WIDTH * mp;
            shift[1].y = sinf(angle + 1.5707963f) * GPS_LINE_WIDTH * mp;
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

        shift[0].x = cosf(angle - 1.5707963f) * GPS_LINE_WIDTH;
        shift[0].y = sinf(angle - 1.5707963f) * GPS_LINE_WIDTH;
        shift[1].x = cosf(angle + 1.5707963f) * GPS_LINE_WIDTH;
        shift[1].y = sinf(angle + 1.5707963f) * GPS_LINE_WIDTH;
    }
    else {
        float mp = FrontEndMenuManager.m_fMapZoom - 140.0f;
        if (mp < 140.0f)
            mp = 140.0f;
        else if (mp > 960.0f)
            mp = 960.0f;
        mp = mp / 960.0f + 0.4f;
        shift[0].x = cosf(angle - 1.5707963f) * GPS_LINE_WIDTH * mp;
        shift[0].y = sinf(angle - 1.5707963f) * GPS_LINE_WIDTH * mp;
        shift[1].y = sinf(angle + 1.5707963f) * GPS_LINE_WIDTH * mp;
        shift[1].x = cosf(angle + 1.5707963f) * GPS_LINE_WIDTH * mp;
    }

    this->Log("DIR: " + std::to_string(dir.x) + ", " + std::to_string(dir.y));

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
        GetD3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    }

    gpsDistance += DistanceBetweenPoints(this->PlayerPos, ThePaths.GetPathNode(resultNodes[0])->GetNodeCoors());
    gpsShown = true;
}

// Check whether on BMX, will always return false if bmx support is enabled.
bool GPSLine::CheckBMX() {
    if (ENABLE_BMX)
        return false;
    
    return FindPlayerPed(0)->m_pVehicle->m_nVehicleSubClass == VEHICLE_BMX;
}

void GPSLine::Run() {
    // Logging stuff
    this->logfile.open("SA.GPS.LOG.txt", std::ios::out);

    // Load config values from file.
    iniFile.open("SA.GPS.CONF.ini", std::ios::in);
    iniParser.parse(iniFile);
    this->Log("INI config loaded:");
    iniParser.generate(this->logfile);
    iniParser.strip_trailing_comments();
    iniParser.interpolate();
    this->Log("INI config processed:");
    iniParser.generate(this->logfile);

    inipp::get_value(iniParser.sections["Navigation Config"], "Navigation line width", GPS_LINE_WIDTH);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation on bicycles", ENABLE_BMX);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation on boats", ENABLE_WATER_GPS);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation for moving targets", ENABLE_MOVING);
    inipp::get_value(iniParser.sections["Navigation Config"], "Navigation line removal proximity", DISABLE_PROXIMITY);

    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line red", GPS_LINE_R);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line green", GPS_LINE_G);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line blue", GPS_LINE_B);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line opacity", GPS_LINE_A);

    inipp::get_value(iniParser.sections["Custom Colors"], "Enabled", ENABLE_CUSTOM_CLRS);

    // Parse custom colors
    if (ENABLE_CUSTOM_CLRS) {
        this->Log("Custom colors enabled.");
        std::string buffer;

        inipp::get_value(iniParser.sections["Custom Colors"], "Red", buffer);
        CC_RED = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "Green", buffer);
        CC_GREEN = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "Blue", buffer);
        CC_BLUE = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "White", buffer);
        CC_WHITE = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "Yellow", buffer);
        CC_YELLOW = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "Purple", buffer);
        CC_PURPLE = this->ExtractColorFromString(buffer);

        inipp::get_value(iniParser.sections["Custom Colors"], "Cyan", buffer);
        CC_CYAN = this->ExtractColorFromString(buffer);
    }

    iniFile.close();

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

    /*
      Clears target blip when player reaches it.
    */

    plugin::Events::gameProcessEvent += [this]() {
        this->UpdatePlayerPos();

        if (FrontEndMenuManager.m_nTargetBlipIndex
            && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
            && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay
            && FindPlayerPed(0)
            && DistanceBetweenPoints(CVector2D(this->PlayerPos),
                CVector2D(CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos)) <= DISABLE_PROXIMITY)
        {
            this->once = false;
            CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
            FrontEndMenuManager.m_nTargetBlipIndex = 0;
        }
    };

    plugin::Events::drawRadarOverlayEvent += [this]() {
        CPed* playa = FindPlayerPed(0);
        if (playa
            && playa->m_pVehicle
            && playa->m_nPedFlags.bInVehicle
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI
            && playa->m_nStatus != STATUS_REMOTE_CONTROLLED
            && !CheckBMX()
            && FrontEndMenuManager.m_nTargetBlipIndex
            && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
            && CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplay)
        {
            CVector destPosn = CRadar::ms_RadarTrace[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vecPos;
            if (!this->once) {
                this->Log("TARGET POS: " + std::to_string(destPosn.x) + ", " + std::to_string(destPosn.y) + ", " + std::to_string(destPosn.z));
                this->once = true;
            }
            this->targetRouteShown = false;
            this->calculatePath(destPosn, targetNodesCount, t_ResultNodes, t_NodePoints, t_NodeHeights, targetDistance);
            this->renderPath(destPosn, -1, false, targetNodesCount, targetRouteShown, t_ResultNodes, t_NodePoints, t_NodeHeights, targetDistance, t_LineVerts);
        }

        if (playa
            && playa->m_pVehicle
            && playa->m_nPedFlags.bInVehicle
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI
            && !CTheScripts::bMiniGameInProgress
            && !CheckBMX()
            && CTheScripts::IsPlayerOnAMission())
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
                    && DistanceBetweenPoints(this->PlayerPos, trace.m_vecPos) > DISABLE_PROXIMITY
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
    };
}

#ifdef SAMP
LPVOID WINAPI GPSLine::init(LPVOID* lpParam) {
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

GPSLine::GPSLine() {
#ifdef SAMP
    this->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GPSLine::init, (LPVOID)this, 0, (LPDWORD)NULL);
#else
    this->Run();
#endif
}



GPSLine::~GPSLine() {
this->logfile.close();
#ifdef SAMP
    if (this->hThread != NULL)
        TerminateThread(this->hThread, 0);
#endif
}

void GPSLine::renderMissionTrace(tRadarTrace trace) {
    this->Log("Found mission objective blip.");

    CVector destVec;
    switch (trace.m_nBlipType) {
    case 1:
        if (ENABLE_MOVING)
            destVec = CPools::GetVehicle(trace.m_nEntityHandle)->GetPosition();
        else
            return;
        break;
    case 2:
        if(ENABLE_MOVING)
            destVec = CPools::GetPed(trace.m_nEntityHandle)->GetPosition();
        else
            return;
        break;
    case 3:
        destVec = CPools::GetObject(trace.m_nEntityHandle)->GetPosition();
        break;
    case 6: // Searchlights
    case 8: // Airstripts
    case 0: // NONE???
        return;
    case 7: // Pickups
        this->Log("Pickup detected. Not providing GPS navigation!");
        return;
    default:
        destVec = trace.m_vecPos;
        break;
    }

    this->Log("DestVec: " + std::to_string(destVec.x) + ", " + std::to_string(destVec.y));
    this->missionRouteShown = false;
    this->calculatePath(destVec, missionNodesCount, m_ResultNodes, m_NodePoints, m_NodeHeights, missionDistance);
    this->renderPath(destVec, trace.m_nColour, trace.m_bFriendly, missionNodesCount, missionRouteShown, m_ResultNodes, m_NodePoints, m_NodeHeights, missionDistance, m_LineVerts);
}

void GPSLine::Log(std::string val) {
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

const char* GPSLine::VectorToString(std::vector<tRadarTrace>& vec) {
    std::string out;
    for (unsigned short i = 0; i < (unsigned short)vec.size() - 1; i++) {
        out += std::to_string((int)vec.at(i).m_nRadarSprite) + ", " + std::to_string(DistanceBetweenPoints(this->PlayerPos, vec.at(i).m_vecPos)) + "\n\t";
    }
    return out.c_str();
}

CRGBA GPSLine::ExtractColorFromString(std::string in) {
    // Remove whitespace
    in.erase(std::remove_if(in.begin(), in.end(), isspace), in.end());

    size_t pos = 0;
    unsigned char 
        R = 0, 
        G = 0,
        B = 0,
        A = 0
    ;
    
    bool 
        didR = false, 
        didG = false,
        didB = false,
        didA = false
    ;

    for (unsigned char i = 0; i < 4; i++) {
        pos = in.find(",");

        if (!didR) {
            R = (unsigned char)std::stoi(in.substr(0, pos));
            didR = true;
        }
        else if (!didG) {
            G = (unsigned char)std::stoi(in.substr(0, pos));
            didG = true;
        }
        else if (!didB) {
            B = (unsigned char)std::stoi(in.substr(0, pos));
            didB = true;
        }
        else if (!didA) {
            A = (unsigned char)std::stoi(in.substr(0, pos));
            didA = true;
        }

        in.erase(0, pos + 1);
    }
    
    return CRGBA(R, G, B, A);
}