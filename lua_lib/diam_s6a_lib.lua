local s6a = {}

local diam = require "diam_lib"

s6a._3GPP_VENDOR_ID = 10415;

s6a.s6a_avp_codes = {};
s6a.s6a_avp_codes.SUPPORTED_FEATURES = 628;
s6a.s6a_avp_codes.SUSCRIPTION_DATA = 1400;
s6a.s6a_avp_codes.APN_CONFIGURATION_PROFILE = 1429;
s6a.s6a_avp_codes.APN_CONFIGURATION = 1430;
s6a.s6a_avp_codes.EPS_SUSCRIBED_QOS_PROFILE = 1431;
s6a.s6a_avp_codes.QOS_CLASS_IDENTIFIER = 1028;
s6a.s6a_avp_codes.EPS_LOCATION_INFORMATION = 1496;

s6a.s6a_dict = {};
s6a.s6a_dict[s6a.s6a_avp_codes.SUPPORTED_FEATURES] =
	{name = "Supported-Features", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.SUSCRIPTION_DATA] =
	{name = "Subscription-Data", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.APN_CONFIGURATION_PROFILE] =
	{name = "Configuration-Profile", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.APN_CONFIGURATION] =
	{name = "APN-Configuration", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.EPS_SUSCRIBED_QOS_PROFILE] =
	{name = "EPS-Subscribed-QoS-Profile", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.QOS_CLASS_IDENTIFIER] =
	{name = "QoS-Class-Identifier", data_type = "int32", flags = diam.VENDOR_AND_MAN_FLAG};
s6a.s6a_dict[s6a.s6a_avp_codes.EPS_LOCATION_INFORMATION] =
	{name = "EPS-Location-Information", data_type = "grouped", flags = diam.VENDOR_FLAG};


function s6a.register_dictionary(dict)
	dict[s6a._3GPP_VENDOR_ID] = s6a.s6a_dict;
end

return s6a;
