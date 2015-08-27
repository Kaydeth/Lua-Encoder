local diam_3gpp_lib = {};

diam_3gpp_lib.VENDOR_ID_3GPP = 10415;

diam_3gpp_lib.avp_codes = {};
diam_3gpp_lib.avp_codes.SUPPORTED_FEATURES = 628;
diam_3gpp_lib.avp_codes.SUBSCRIPTION_DATA = 1400;
diam_3gpp_lib.avp_codes.APN_CONFIGURATION_PROFILE = 1429;
diam_3gpp_lib.avp_codes.APN_CONFIGURATION = 1430;
diam_3gpp_lib.avp_codes.EPS_SUBSCRIBED_QOS_PROFILE = 1431;
diam_3gpp_lib.avp_codes.QOS_CLASS_IDENTIFIER = 1028;
diam_3gpp_lib.avp_codes.EPS_LOCATION_INFORMATION = 1496;

local VENDOR_FLAG = 128;
local VENDOR_AND_MAN_FLAG = 192;

diam_3gpp_lib.avp_dict = {};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.SUPPORTED_FEATURES] =
	{name = "Supported-Features", data_type = "grouped", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.SUBSCRIPTION_DATA] =
	{name = "Subscription-Data", data_type = "grouped", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.APN_CONFIGURATION_PROFILE] =
	{name = "Configuration-Profile", data_type = "grouped", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.APN_CONFIGURATION] =
	{name = "APN-Configuration", data_type = "grouped", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.EPS_SUBSCRIBED_QOS_PROFILE] =
	{name = "EPS-Subscribed-QoS-Profile", data_type = "grouped", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.QOS_CLASS_IDENTIFIER] =
	{name = "QoS-Class-Identifier", data_type = "int32", flags = VENDOR_AND_MAN_FLAG};
diam_3gpp_lib.avp_dict[diam_3gpp_lib.avp_codes.EPS_LOCATION_INFORMATION] =
	{name = "EPS-Location-Information", data_type = "grouped", flags = VENDOR_FLAG};

return diam_3gpp_lib;
