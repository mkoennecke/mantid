# Setup common things for the Vates subprojects

include ( SetMantidSubprojects )

set_mantid_subprojects (
Framework/API
Framework/Geometry
Framework/Kernel
Framework/DataObjects
Framework/MDEvents
)

set ( COMMONVATES_SETUP_DONE TRUE )
