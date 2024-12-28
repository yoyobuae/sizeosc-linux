/* 
    Example of two different ways to process received OSC messages using oscpack.
    Receives the messages from the SimpleSend.cpp example.
*/

#include <iostream>
#include <cstring>
#include <memory>

#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
using ::__strcmp__;  // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include "osc/OscPacketListener.h"
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

// OpenVR to interact with VR
#include <openvr.h>

#define ADDRESS "127.0.0.1"
#define PORT 9000

#define LISTEN_PORT 9430

#define OUTPUT_BUFFER_SIZE 1024

using namespace vr;

namespace vr {
    static const char * const k_pch_VRChat_Section = "steam.app.438100";
}

static bool isVRValid = false;
static float osc_scale_min = 0.5034f;
static float osc_scale_base = 1.0f;
static float osc_scale_max = 9.0f;

float mapScale(float value)
{
    if( value <= 0.5f )
    {
        float t = 2 * value;
        return (1.0f - t) * osc_scale_min + t * osc_scale_base;
    }
    else
    {
        float t = 2 * (value - 0.5f);
        return (1.0f - t) * osc_scale_base + t * osc_scale_max;
    }
}

class PingHandler : public osc::OscPacketListener {
protected:

    virtual void ProcessMessage(const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint)
    {
        try{
            // example of parsing single messages. osc::OsckPacketListener
            // handles the bundle traversal.
            
            if( std::strcmp( m.AddressPattern(), "/avatar/parameters/SizeOSC_Scale_Puppet" ) == 0 ){
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();

                struct timespec clientSendTime, serverReceiveTime;
                clock_gettime(CLOCK_MONOTONIC, &serverReceiveTime);

                float value = 0.0f;

                args >> value >> osc::EndMessage;

                std::cout << "SizeOSC_Scale_Puppet received: " << value << " sending: " << (value * 2) - 1.0f << " VR: " << 1/mapScale(value) << std::endl;

                if (isVRValid)
                    vr::VRSettings()->SetFloat(vr::k_pch_VRChat_Section, vr::k_pch_SteamVR_WorldScale_Float, 1/mapScale(value));

                char buffer[OUTPUT_BUFFER_SIZE];
                osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );

                p << osc::BeginBundleImmediate
                    << osc::BeginMessage( "/avatar/parameters/SizeOSC_Scale" ) 
                    << (value * 2 - 1.0f) << osc::EndMessage
                    << osc::EndBundle;

                UdpTransmitSocket transmitSocket(IpEndpointName(ADDRESS, PORT));
                transmitSocket.Send( p.Data(), p.Size() );

            }
        }catch( osc::Exception& e ){
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};

void shutdown_vr(vr::IVRSystem *_system)
{
    vr::VR_Shutdown();
}


int main(int argc, char* argv[])
{
    (void) argc; // suppress unused parameter warnings
    (void) argv; // suppress unused parameter warnings

    PingHandler listener;
    UdpListeningReceiveSocket s(
            IpEndpointName( IpEndpointName::ANY_ADDRESS, LISTEN_PORT ),
            &listener );

    EVRInitError init_error;
    std::unique_ptr<IVRSystem, decltype(&shutdown_vr)>
        system(VR_Init(&init_error, VRApplication_Background), &shutdown_vr);
    if (init_error)
    {
        std::cout << VR_GetVRInitErrorAsEnglishDescription(init_error) << std::endl;
        isVRValid = false;
    }
    else
    {
        isVRValid = true;
    }

    std::cout << "press ctrl-c to end" << std::endl;

    s.RunUntilSigInt();

    return 0;
}

