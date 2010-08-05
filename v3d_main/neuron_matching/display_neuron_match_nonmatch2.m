function display_neuron_match_nomatch2(a, b, matchingList, color_a_match, color_b_match, color_a_nonmatch, color_b_nonmatch)
% function display_neuron_match(a, b, matchtag)

% matchtag = 1: display lines between matched nodes
% Display the neuron structure matching result
% by F. Long
% 20090115

% h = figure(1);
hold on;

visited_a = zeros(1,size(a,1));
visited_b = zeros(1,size(b,1));

% for i=1:size(matchingList,1)
%     
%     if (matchingList(i,2)>=0)
%         
%         idx1 = find(a(:,1)==matchingList(i,1));
%         idx2 = find(b(:,1)==matchingList(i,2));
% %         [idx1, idx2]
%         
%         % plot the line connecting matching points
%         d = [a(idx1,3:5); b(idx2,3:5)];
% 
% %         if (matchingList(i,1)==matchingList(i,2))    
% %             plot3(d(:,1), d(:,2), d(:,3), 'color', [0,1,0]);
% %         else
% %             plot3(d(:,1), d(:,2), d(:,3), 'color', [1,0,0]);
% %         end;
% 
% 
% %         plot3(d(:,1), d(:,2), d(:,3), '-','color', [0,1,0]);
%         plot3(d(:,1), d(:,2), d(:,3), '-','color', [1,0,0]);
% 
%     end
% end;


for i=1:size(matchingList,1)
    
    if (matchingList(i,2)>=0)
        
        idx1 = find(a(:,1)==matchingList(i,1));
        
        % plot matching abranches in neuron a using different colors
        
        if (a(idx1,7)<0), % root
            continue;
        end;

        nn1=a(idx1,3:5); 
       
        while (1)
            
            visited_a(idx1) = 1;
            
            idxparent = find(a(:,1)==a(idx1,7));            
            
%             tt = nnz(a(:,7) == a(idx1,7));
            
            
%             if (isempty(idxparent)&(tt>1)) % root
%                 tt
            if (isempty(idxparent)) % root
                break;
            end;
            
            nn2 = a(idxparent, 3:5);
            nn=[nn1;nn2];
            c=a(idxparent,6)*1; 

            h=plot3(nn(:,1), nn(:,2), nn(:,3), '.-', 'color', color_a_match); 
            set(h, 'linewidth', c); 

            ind = find(matchingList(:,1)==idxparent);
            
            if (~isempty(ind)&(length(ind)>1)) % if parent node is in the matching list, then stop, otherwise keep on searching for a parent that is in the matching list
                break;
            end;
            
            idx1 = idxparent;
            nn1 = nn2;
            
        end;
    end;
end;

for i=1:size(matchingList,1)

    if (matchingList(i,2)>=0)
        idx2 = find(b(:,1)==matchingList(i,2));

%     idx2 = i;

        % plot matching branches in neuron b using different colors

        if (b(idx2,7)<0), % root
            continue;
        end;

        % plot 
        nn1=b(idx2,3:5); 

        while (1)

            visited_b(idx2) = 1;

            idxparent = find(b(:,1)==b(idx2,7));            

            if (isempty(idxparent)) % root
                break;
            end;

            nn2 = b(idxparent, 3:5);
            nn=[nn1;nn2];
            c=b(idxparent,6)*1; 

            h=plot3(nn(:,1), nn(:,2), nn(:,3), '.-', 'color', color_b_match); 
            set(h, 'linewidth', c); 

            ind = find(matchingList(:,2)==idxparent);

%            if (~isempty(ind)) % if parent node is in the matching list, then stop, otherwise keep on searching for a parent that is in the matching list
            if (~isempty(ind)&(length(ind)>1))
                break;
            end;

            idx2 = idxparent;
            nn1 = nn2;

        end;
    end;

end;

grid on;
% plot the remaining non-matched branches 
idx = setdiff([1:size(a,1)], find(visited_a>0));

for i=1:length(idx)

    nn1=a(idx(i),3:5); 
    
   idxparent = find(a(:,1)==a(idx(i),7));            

    if (isempty(idxparent)) % root
        continue;
    end;

    nn2 = a(idxparent, 3:5);
    nn=[nn1;nn2];
    c=a(idxparent,6)*1; 

    h=plot3(nn(:,1), nn(:,2), nn(:,3), '.-', 'color', color_a_nonmatch); 
    set(h, 'linewidth', c); 
    

end;


idx = setdiff([1:size(b,1)], find(visited_b>0));

for i=1:length(idx)
    
    nn1=b(idx(i),3:5); 
    
   idxparent = find(b(:,1)==b(idx(i),7));            

    if (isempty(idxparent)) % root
        continue;
    end;

   nn2 = b(idxparent, 3:5);
    nn=[nn1;nn2];
    c=b(idxparent,6)*1; 

    h=plot3(nn(:,1), nn(:,2), nn(:,3), '.-', 'color', color_b_nonmatch); 
    set(h, 'linewidth', c);     

end;

 for i=1:size(matchingList,1)
    
    if (matchingList(i,2)>=0)
        
        idx1 = find(a(:,1)==matchingList(i,1));
        idx2 = find(b(:,1)==matchingList(i,2));
%         [idx1, idx2]
        
        % plot the line connecting matching points
        d = [a(idx1,3:5); b(idx2,3:5)];

%         if (matchingList(i,1)==matchingList(i,2))    
%             plot3(d(:,1), d(:,2), d(:,3), 'color', [0,1,0]);
%         else
%             plot3(d(:,1), d(:,2), d(:,3), 'color', [1,0,0]);
%         end;


          plot3(d(:,1), d(:,2), d(:,3), '-','color', [0,1,0]);
%          plot3(d(:,1), d(:,2), d(:,3), '-','color', [1,0,0]);
        matchingList(i,:)
%           pause;
    end
end;


