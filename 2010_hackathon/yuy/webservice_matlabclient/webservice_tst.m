
%%
% ex1

% message = createSoapMessage(...
% 'urn:v3dwebserver',... % Relative path to namespace of web service on local intranet
% 'helloworld',... % Method (operation) provided by service
% {'Hanchuan'},... % Input that method requires
% {'name'},... % Name of paremeter
% {'{http://www.w3.org/2001/XMLSchema}string'},... % Data type for the result
% 'rpc') % SOAP message style
% 
% response = callSoapService('http://127.0.0.1:9125',... % Service's endpoint
% 'urn:demowebserver#helloworld',... % Server method to run
% message) % SOAP message created using createSoapMessage


%%
% ex2

% % data struct
% ex_cell.x = 128;
% ex_cell.y = 128;
% ex_cell.z = 128;
% ex_cell.c = 3;
% ex_cell.t = 1;
% ex_cell.intensity = 0.0;
% ex_cell.dt = 0; % 0 UINT8 1 UINT16 3 FLOAT32
% 
% message = createSoapMessage(...
% 'urn:v3dwebserver',... % Relative path to namespace of web service on local intranet
% 'msghandler',... % Method (operation) provided by service
% {ex_cell},... % Input that method requires
% {'lm'}... % Name of paremeter
% )
% 
% 
% response = callSoapService('http://127.0.0.1:9125',... % Service's endpoint
% 'urn:demowebserver#msghandler',... % Server method to run
% message) % SOAP message created using createSoapMessage

%%
% ex3

function [result] = webservice_est(para)

message = createSoapMessage(...
'urn:v3dwebserver',... % Relative path to namespace of web service on local intranet
'v3dopenfile',... % Method (operation) provided by service
{para},... % Input that method requires '/Users/yuy/misc/jichao2010/measurement/diameter_est/dataset/yjz/OPT41c1.tif'
{'fn'},... % Name of paremeter
{'{http://www.w3.org/2001/XMLSchema}string'},... % Data type for the result
'rpc') % SOAP message style

response = callSoapService('http://127.0.0.1:9125',... % Service's endpoint
'urn:demowebserver#v3dopenfile',... % Server method to run
message) % SOAP message created using createSoapMessage

result = parseSoapResponse(response)