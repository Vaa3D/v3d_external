function display_neuron_swcformat_FL(a, b_newfig, overlay_color)
%function display_neuron_swcformat(a, b_newfig, overlay_color)
% Display the neuron structure in SWC format
% by Hanchuan Peng
% 070726
% 070809: add a b_newfig option
% add overlay_color option
%080225: do not display the unused" nodes which are labeled by a -2 (<0) parent node

if nargin<3,
    overlay_color = -1;
else,
    overlay_color = overlay_color(:);
end;
N_overlay_color = length(overlay_color);

if nargin<2,
    b_newfig=1;
end;

if b_newfig,
    figure; 
end;

hold on; 
for i=1:size(a,1), %FL, 20090114 

% for i=1:length(a), 
    
    if (a(i,7)<0), % root
        continue;
    end;
    
    b1=a(i,3:5); 
%     b2 = a(a(i,7), 3:5);
    
    idx = find(a(:,1)==a(i,7));    
    b2=a(idx, 3:5); 
    
    c=a(i,6)*1; 
    d=[b1;b2]; 
    h=plot3(d(:,1), d(:,2), d(:,3),'.-'); 
    %pause(0.5)
    set(h, 'linewidth', c); 
    
    if (overlay_color(1)~=-1 & N_overlay_color==3),
        set(h, 'color', overlay_color);
    else,
        switch a(i,2),
            case 1,
                set(h, 'color', [0 0 0]); %% set soma color as black
            case 2,
                set(h, 'color', [1 0 0]); %% set axon color to be red
            case 3,
                set(h, 'color', [0 0 1]); %%set (basal) dendrite color to be blue
            case 4, 
                set(h, 'color', [1 0 1]); %%all apical dendrite color to be purple 
            otherwise,
                set(h, 'color', [0 1 1]); %%otherwise 
        end;
    end;
end;
axis image;
grid on;
xlabel('x'); ylabel('y'); zlabel('z');


hold off;
