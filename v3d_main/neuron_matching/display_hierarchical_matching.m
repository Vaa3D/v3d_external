function display_hierarchical_matching(a, b, matchingList, matchingList_allterms, filedir)
% function display_hierarchical_matching(a, b, matchingList, matchingList_allterms, color_a_match, color_b_match, color_a_nonmatch, color_b_nonmatch)

% display hierarchical matching of branching points and leaf nodes
% by F. Long
% 20090317

% h = figure(1);
% hold on;


% % -------------------------------------
% % plot neurons 
% % -------------------------------------
% 
% 
% display_neuron_swcformat_FL(a, 1, [0 0 0]); 
% display_neuron_swcformat_FL(b, 0, [0 0 1]);
% hold on;
% 
% %-------------------------------
% % plot matched nodes
% %-------------------------------
% 
 maxlevel = max(matchingList_allterms(:,end));
 levelcolor = [1,0,0; 0,1,0; 0,0,1; 0,0,0];
%  
%  for level = 0:maxlevel
%      for i=1:size(matchingList_allterms,1)
% 
%         if (matchingList_allterms(i,end)==level)
% 
%             idx1 = find(a(:,1)==matchingList_allterms(i,1));
%             idx2 = find(b(:,1)==matchingList_allterms(i,2));
%     %         [idx1, idx2]
% 
%             % plot the line connecting matching points
%             d = [a(idx1,3:5); b(idx2,3:5)];
% 
%     %         if (matchingList(i,1)==matchingList(i,2))    
%     %             plot3(d(:,1), d(:,2), d(:,3), 'color', [0,1,0]);
%     %         else
%     %             plot3(d(:,1), d(:,2), d(:,3), 'color', [1,0,0]);
%     %         end;
% 
% 
%     %           plot3(d(:,1), d(:,2), d(:,3), '-','color', [0,1,0]);
%              plot3(d(:,1), d(:,2), d(:,3), '-','color', levelcolor(level+1,:));
%              matchingList_allterms(i,:)
%         end
%     end;
%     pause;
%  end;
%  
%-------------------------------
% plot matched subtrees
%-------------------------------

branchcolor = [1,0,0; 0,1,0; 0 0 1; 1 1 0; 1 0 1; 0 1 1];

for level = 0:maxlevel-1
    figure;
    
    % plot matching branches
    branchmatchlist = load([filedir, 'branchmatch_level', num2str(level), '.txt'], 'ASCII');
        
    for i=1:size(branchmatchlist,1)
        branch_idx1 = branchmatchlist(i,1);
        branch_idx2 = branchmatchlist(i,2);

        c = load([filedir, 'Tree2_level', num2str(level), '_branch', num2str(branch_idx2), '.swc'], 'ASCII');
        d = load([filedir, 'Tree1_level', num2str(level), '_branch', num2str(branch_idx1), '.swc'], 'ASCII');

        j = mod(i,6);
        if (j==0)
            j = 6;
        end;
        
        display_neuron_swcformat_FL(c, 0, branchcolor(j,:)); 
        display_neuron_swcformat_FL(d, 0, branchcolor(j,:));

    end;

    hold on;
    
    % plot un-matched branches of Tree1
    % find the maximum index of branches
    maxnum = max(branchmatchlist(:,1))+1;
    
    while (1)
        filename = [filedir, 'Tree1_level', num2str(level), '_branch', num2str(maxnum), '.swc'];
        if (~exist(filename, 'file'))
            break;
        end;
        maxnum = maxnum+1;
    end;
    maxnum = maxnum-1;

    unmatchedidx = setdiff([1:maxnum], branchmatchlist(:,1));
    
    for i=1:length(unmatchedidx)
        d = load([filedir, 'Tree1_level', num2str(level), '_branch', num2str(unmatchedidx(i)), '.swc'], 'ASCII');
        display_neuron_swcformat_FL(d, 0, [0 0 0]); 
    end;
 
    % plot un-matched branches of Tree2
    % find the maximum index of branches
    maxnum = max(branchmatchlist(:,2))+1;
    
    while (1)
        filename = [filedir, 'Tree2_level', num2str(level), '_branch', num2str(maxnum), '.swc'];
        if (~exist(filename, 'file'))
            break;
        end;
        maxnum = maxnum+1;

    end;
    maxnum = maxnum-1;

    unmatchedidx = setdiff([1:maxnum], branchmatchlist(:,2));
    
    for i=1:length(unmatchedidx)
        c = load([filedir, 'Tree2_level', num2str(level), '_branch', num2str(unmatchedidx(i)), '.swc'], 'ASCII');
        display_neuron_swcformat_FL(c, 0, [0 0 0]); 
    end;    
    
    hold on;
    
    % plot matching points
     for i=1:size(matchingList_allterms,1)

        if (matchingList_allterms(i,end)<=level)

            idx1 = find(a(:,1)==matchingList_allterms(i,1));
            idx2 = find(b(:,1)==matchingList_allterms(i,2));
    %         [idx1, idx2]

            % plot the line connecting matching points
            d = [a(idx1,3:5); b(idx2,3:5)];

             plot3(d(:,1), d(:,2), d(:,3), '-','color', levelcolor(level+1,:)); hold on;
             matchingList_allterms(i,1:2)
%              pause;
        end
    end;
%     pause;
    
end;
    
% plot matched leaf nodes 


figure;

% plot matching branches
branchmatchlist = load([filedir, 'branchmatch_level', num2str(level), '.txt'], 'ASCII');

for i=1:size(branchmatchlist,1)
    branch_idx1 = branchmatchlist(i,1);
    branch_idx2 = branchmatchlist(i,2);

    c = load([filedir, 'Tree2_level', num2str(level), '_branch', num2str(branch_idx2), '.swc'], 'ASCII');
    d = load([filedir, 'Tree1_level', num2str(level), '_branch', num2str(branch_idx1), '.swc'], 'ASCII');

    j = mod(i,6);
    if (j==0)
        j = 6;
    end;

    display_neuron_swcformat_FL(c, 0, branchcolor(j,:)); 
    display_neuron_swcformat_FL(d, 0, branchcolor(j,:));

end;

hold on;
    
 for i=1:size(matchingList_allterms,1)

    if (matchingList_allterms(i,end)>=0)

        idx1 = find(a(:,1)==matchingList_allterms(i,1));
        idx2 = find(b(:,1)==matchingList_allterms(i,2));
%         [idx1, idx2]

        % plot the line connecting matching points
        d = [a(idx1,3:5); b(idx2,3:5)];

         plot3(d(:,1), d(:,2), d(:,3), '-','color', levelcolor(maxlevel+1,:)); hold on;
         matchingList_allterms(i,1:2)
%          pause;
    end
end;
%     pause;


