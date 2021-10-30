function display_neuron_match(a, b, matchingList)
% function display_neuron_match(a, b, matchtag)

% matchtag = 1: display lines between matched nodes
% Display the neuron structure matching result
% by F. Long
% 20090115

% h = figure(1);
hold on;

for i=1:size(matchingList,1)
    
    if (matchingList(i,2)>=0)
        
        idx1 = find(a(:,1)==matchingList(i,1));
        idx2 = find(b(:,1)==matchingList(i,2));
        [idx1, idx2]
        
        d = [a(idx1,3:5); b(idx2,3:5)];

        plot3(d(:,1), d(:,2), d(:,3), 'color', [0,1,0]);
        
%         if (matchingList(i,1)==matchingList(i,2))    
%             plot3(d(:,1), d(:,2), d(:,3), 'color', [0,1,0]);
%         else
%             plot3(d(:,1), d(:,2), d(:,3), 'color', [1,0,0]);
%         end;
        pause;
    end;

end;



