function gen_swc4v3d_hierarchical_matching(a, b, matchingList_allterms, filedir)
% function gen_swc4v3d_hierarchical_matching(a, b, matchingList_allterms, filedir)

% generate swc file for v3d to display hierarchical matching result
% tree1, tree2, and lines indicating the matching branching points and
% leaves are all combined into one tree called v3dtree for v3d to display
%
% a is tree1, b is tree2
% matchingLit_allterms is the matching branching points
%
% by F. Long
% 20090317


 
maxlevel = max(matchingList_allterms(:,end)); % number of levels
labelshift = 1000; % 

%-------------------------------
% plot matched subtrees
%-------------------------------


for level = 0:maxlevel-1

    v3dtree = []; %the swc tree that combines tree1, tree2, and lines between matching branching points and leaves
    lenold = size(v3dtree,1);

    branchmatchlist = load([filedir, 'branchmatch_level', num2str(level), '.txt'], 'ASCII'); %indicate how branche are matched at the current level
    level  

    rootidx = []; % list the index of roots of sub-branches in v3dtree
    rootmap1 = []; % map the index of roots of sub-branches of tree1 in v3dtree and their original nodeid
    rootmap2 = []; % map the index of roots of sub-branches of tree2 in v3dtree and their original nodeid
    
    nodeBranchIdx = [];
    
    for i=1:size(branchmatchlist,1)
        
        branch_idx1 = branchmatchlist(i,1);
        branch_idx2 = branchmatchlist(i,2);
        
        c = load([filedir, 'Tree2_level', num2str(level), '_branch', num2str(branch_idx2), '.swc'], 'ASCII'); % b and c correspond to tree 2
        d = load([filedir, 'Tree1_level', num2str(level), '_branch', num2str(branch_idx1), '.swc'], 'ASCII'); % a and d correspond to tree 1

        listidx1 = []; % record roots of sub-branches in tree2
        listidx3 = []; % record roots of sub-branches in tree1
        
        % determine color of ech sub-branch
%         j = mod(i,8); % 0: white; 1:black; 2:red; 3:blue; 4:purple; 5:cyan; 6:yellow; 7:green; 8:coffee
%         if (j==0)
%             j = 8;
%         end;
        
        j = mod(i,7); % 0: white; 1:black; 2:red; 3:blue; 4:purple; 5:cyan; 6:yellow; 7:green; 8:coffee
        if (j==0)
            j = 7;
        end;

        % make one of the neuron thicker
% %         c(:,6) = c(:,6)*20;
%         c(:,6) = 0.5;
        
        % revise parentid of branch root in c (tree 2) so that branches are connected in v3d
        idx1 = find(c(:,7)==-1);
        idx2 = find(c(idx1,1)==b(:,1));
        c(idx1,7) = b(idx2,7);
        listidx1 = [listidx1, idx1];
        
        % revise parentid of branch root in d (tree 1) so that branches are connected in v3d
        idx3 = find(d(:,7)==-1);
        idx4 = find(d(idx3,1)==a(:,1));
        d(idx3,7) = a(idx4,7);
        listidx3 = [listidx3, idx3];

        rootmap2= [rootmap2; lenold+listidx1, c(listidx1,1)]; 
        rootmap1 = [rootmap1; lenold+size(c,1)+listidx3, d(listidx3,1)]; 
        
        % make nodeid and parentid of d different from that of c, so that they can be put in the same tree  
                
        d(:,1) = d(:,1)+labelshift; % add shift to nodeid of tree1
        idx = find(d(:,7)~=-1);
        d(idx,7) = d(idx,7)+labelshift;
        
        
        rootidx = [rootidx, lenold+listidx1, lenold+size(c,1)+listidx3]; % order is: branch root in tree2 and then in tree1
        
        v3dtree = [v3dtree; c; d]; 
        % order of v3dtree is: from level 0 to the maximum level: tree2 matched sub-branch, tree1 matched sub-branch, tree1 unmatched sub-branches, tree2 unmatched sub-branches
        % finally the leave level
        
        len = size(v3dtree,1);       
        v3dtree(lenold+1:len,2) = j+1; % avoid using white color, since it will be used to highlight matched branching points
        nodeBranchIdx(lenold+1:len) = i; % recode for each node in v3dtree, which branch (index of branch) it is in. This is useful in determining branch color when two branching points are direct parent-child

        lenold = len;

    end;

    % add un-matched branches of Tree1
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

    unmatchedidx = setdiff([1:maxnum], branchmatchlist(:,1))
    
    for i=1:length(unmatchedidx)
        d = load([filedir, 'Tree1_level', num2str(level), '_branch', num2str(unmatchedidx(i)), '.swc'], 'ASCII');

       % revise parentid of branch root in d so that branches are connected in
        % v3d
        idx3 = find(d(:,7)==-1);
        idx4 = find(d(idx3,1)==a(:,1));
        d(idx3,7) = a(idx4,7);
        
        % make nodeid and parentid of d different from that of c        
        d(:,1) = d(:,1)+labelshift;
        idx = find(d(:,7)~=-1);
        d(idx,7) = d(idx,7)+labelshift;
        
        v3dtree = [v3dtree; d];
        len = size(v3dtree,1);       
        v3dtree(lenold+1:len,2) = 1; % use black color to indicate mismatch, white color to indicate match
        lenold = len;    
        
    end;
    
 
    % add un-matched branches of Tree2
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

    unmatchedidx = setdiff([1:maxnum], branchmatchlist(:,2))
    
    for i=1:length(unmatchedidx)
        c = load([filedir, 'Tree2_level', num2str(level), '_branch', num2str(unmatchedidx(i)), '.swc'], 'ASCII');
        
        %make it thicker
% %         c(:,6) = c(:,6)*20;
%         c(:,6) = 0.5;
 
        % revise parentid of branch root in c so that branches are connected in
        % v3d
        idx1 = find(c(:,7)==-1);
        idx2 = find(c(idx1,1)==b(:,1));
        c(idx1,7) = b(idx2,7);
                
        v3dtree = [v3dtree; c];
        len = size(v3dtree,1);       
        v3dtree(lenold+1:len,2) = 1; % use black color to indicate mismatch, white color to indicate match
        lenold = len;  
        
    end;  
    
    oringinalcolor = v3dtree(:,2);
    
    % set the color of connection point, here need to be very careful in
    % dealing with branching nodes are adjacent (i.e., have direct
    % parent-child relationship)
    
    % old code that has bugs
%     for i=1:length(rootidx)
%         if (v3dtree(rootidx(i),7)~=-1)
%             nn = find(v3dtree(:,1) == v3dtree(rootidx(i),7)); % note length(nn) can be bigger than 1, due to replicate of nodes in v3dtree
% 
%             if (~isempty(find(rootidx == nn(1)))) % the parent node is a branching node, in this case, branch color should be determined by orginal color of parent nodes, not after modification for connection
%                 
% %                 % since length(nn) may be bigger than 1, need to determine
% %                 % which branch (the same node at the root of different
% %                 % branches have different colors)
% %                 
% %                 j = 1;
% %                 while (j<length(nn))
% %                     if (nodeBranchIdx(nn(j))==nodeBranchIdx(rootidx(i)))
% %                         break;
% %                     else
% %                         j = j+1;
% %                     end;
% %                 end;
% %                     
% %                 v3dtree(rootidx(i),2) = oringinalcolor(nn(j));
% 
%                 v3dtree(rootidx(i),2) = nodeBranchIdx(rootidx(i)); % the above is incorrect, fix it on 20090911, incorrect
% 
%             else
%                 v3dtree(rootidx(i),2) = v3dtree(nn(1),2);
%             end;
% 
%         end;
%     end;

    % fix bugs on 20090914
    % load the file indicating for each subtree, what are the starting and
    % ending seednodes. This is needed for rendering branch where its
    % parent node is a significant branching node
    subTreeSeedNodes1 = load([filedir, 'Tree1_branchseeds_level', num2str(level), '.txt'], 'ASCII');     
    subTreeSeedNodes2 = load([filedir, 'Tree2_branchseeds_level', num2str(level), '.txt'], 'ASCII'); 

    subTreeNum1 = size(subTreeSeedNodes1,1);
    subTreeNum2 = size(subTreeSeedNodes2,1);    
    lenn = size(subTreeSeedNodes1,2)

    
    for i=1:size(branchmatchlist,1)
        ii = branchmatchlist(i,1)+1;
        j=2;
        while (j<=lenn)
            if(subTreeSeedNodes1(ii,j)>0)
                
                idx = find(rootmap1(:,2)== subTreeSeedNodes1(ii,j));
                idx = rootmap1(idx,1);

                cc = mod(i,7);
                if (cc==0)
                   cc = 7;
                end;
                
                v3dtree(idx,2) = cc+1; % set the color of the subtree root to the color of the parent branch
                j = j+1;
            else
               break;
            end;
        end;
    end;

    for i=1:size(branchmatchlist,1)
        ii = branchmatchlist(i,2)+1;
        j=2;
        while (j<=lenn)
            if(subTreeSeedNodes2(ii,j)>0)
                
                idx = find(rootmap2(:,2)== subTreeSeedNodes2(ii,j));
                idx = rootmap2(idx,1);

                cc = mod(i,7);
                if (cc==0)
                   cc = 7;
                end;
                
                v3dtree(idx,2) = cc+1; % set the color of the subtree root to the color of the parent branch
                j = j+1;
            else
               break;
            end;
        end;
    end;

    % add lines indicating matching between nodes
     len = size(v3dtree,1);
     
     for i=1:size(matchingList_allterms,1)

        if (matchingList_allterms(i,end)<=level)

            idx1 = find(v3dtree(:,1) == (matchingList_allterms(i,1)+labelshift)); % there can be multiple idx1
            idx2 = find(v3dtree(:,1) == matchingList_allterms(i,2)); % there can be multiple idx2

            nodeid1 = max(v3dtree(:,1))+1;
            nodeid2 = max(v3dtree(:,1))+2;
            
            len = len+1;
            v3dtree(len,:) = v3dtree(idx1(1), :);
            
            v3dtree(len,1) = nodeid1; % add a new node
            v3dtree(len, 2) = 0; % line color
%             v3dtree(len, 2) = v3dtree(len, 2)+0.001; % chnage x coordinate a little bit, to make it a different node
            v3dtree(len, 6) = 0.25; % line thickness
            v3dtree(len,7) = nodeid2;
            
            len = len +1;
            v3dtree(len,:) = v3dtree(idx2(1), :);
            v3dtree(len,1) = nodeid2; % add a new node
%             v3dtree(len, 2) = 0; % line color
            v3dtree(len, 2) = 1; % line color, for recomb paper

            v3dtree(len, 6) = 0.25; % line thickness
           
            v3dtree(len,7) = nodeid1;
           
            
        end
    end;

    saveSwcVariable2File(v3dtree, [filedir, 'v3dtree', num2str(level), '.swc']);
    length(unique(v3dtree(:,1)))    

end;
    
% add lines between matched leaf nodes to the last level, 
% the same as the above, only save to different files
 
 len = size(v3dtree,1);
 
 for i=1:size(matchingList_allterms,1)

    if (matchingList_allterms(i,end)==maxlevel)

        idx1 = find(v3dtree(:,1) == (matchingList_allterms(i,1)+labelshift)); % there can be multiple idx1
        idx2 = find(v3dtree(:,1) == matchingList_allterms(i,2)); % there can be multiple idx2


            nodeid1 = max(v3dtree(:,1))+1;
            nodeid2 = max(v3dtree(:,1))+2;
            
            len = len+1;
            v3dtree(len,:) = v3dtree(idx1(1), :);
            
            v3dtree(len,1) = nodeid1; % add a new node
            v3dtree(len, 2) = 0; % line color
%             v3dtree(len, 2) = v3dtree(len, 2)+0.001; % chnage x coordinate a little bit, to make it a different node
            v3dtree(len, 6) = 0.25; % line thickness
            v3dtree(len,7) = nodeid2;
            
            len = len +1;
            v3dtree(len,:) = v3dtree(idx2(1), :);
            v3dtree(len,1) = nodeid2; % add a new node
%             v3dtree(len, 2) = 0; % line color
            v3dtree(len, 2) = 1; % line color, for recomb paper

            v3dtree(len, 6) = 0.25; % line thickness
           
            v3dtree(len,7) = nodeid1;

    end
end;
% v3dtree(:,6)=v3dtree(:,6)*3;

saveSwcVariable2File(v3dtree, [filedir, 'v3dtree', num2str(maxlevel), '.swc']);

