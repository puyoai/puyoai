import math
import copy 
import random
import sys
import operator 
import urllib
from collections import defaultdict
colors = {-1: '', 0: '', 2: '\033[34m', 4:'\033[92m', 1:'\033[95m', 3:'\033[93m', 5:'\033[91m'}
X = 6 + 2
Y = 13 + 2
xys = [(1+ (x%6)) + (1 + (x/6)) * 8 for x in range(6*13)]

def get_url(field):
	url = ""
	tmpstr = ""
	for x in reversed(xys):
		p = field[x]
		if p == 0:
			tmpstr += "0"
		else:
			url  =  str({0:0, 1:4, 2:5, 3:6, 4:7, 5:8}[p]) + tmpstr + url
			tmpstr = ""
	return "http://www.inosendo.com/puyo/rensim/??" + url

def print_field(field, highlight_pos = None, next_puyos = None):	
	if next_puyos:
		line = ""
		for ps in next_puyos:
			for p in ps:
				line += colors.get(p) + "%2d" % p + '\033[0m'
			line += " -> "
		print line	
	for y in range(Y):
		line = ""
		for x in range(X):
			p = field[x+y*8]
			if highlight_pos and any([pos == x+y*8 for pos in highlight_pos]):
				line += "\033[40m"
			line +=  colors.get(p) + "%2d" % p + '\033[0m' 
		print line


def get_connections(field):
	visited = [0]*X*Y 
	ret = []
	for x in xys:
		if field[x] <= 0 or visited[x]:
			continue
		connected = get_connected(field, x, visited)
		ret.append(connected)
	return ret

HOGE = 0
def get_connected(field, x, visited = None):
	if visited == None:
		visited = [] 

	p = field[x]
	s = [x]
	count = 0
	connected = []
	while s:
		x = s.pop()
		if visited[x]:
			continue
		visited[x] = True
		connected.append(x)
		for dx in (-1,1,8,-8):
			xx = dx + x
			if field[xx] == p and not visited[xx]:
				s.append(xx)
	return connected  

def vanish(field, vanished):
	dys = [0]*X*Y
	for x in vanished:
                field[x] = 0
		for x in range(x-8, 0, -8):
			dys[x] += 1
	#print_field(field, vanished)
        #print_field(dys)
	for x in reversed(xys):
		dy = dys[x]
		if dy >  0:
			field[x+dy*8] = field[x]
	for x in range(X):
		for y in range(dys[x]):
			field[x+(y+1)*8] = 0

rensa_bonus= [0,8,16,32,64,96,128,160,192,224,256,288,320,352,388,416,448,480,512]
doujikesi_bonus=[0,0,0,0,0,2,3,4,5,6,7,10] + [10]*100
doujikesi_color_bonus=[0,0,3,6,12,24]

def fire(field, fire_from=None, connections=None):
	n = 0
        total_score = 0	
	if fire_from:
                #print "fire_from:", fire_from
		vanish(field, fire_from)
		n += 1
	while True:
                score = rensa_bonus[n] 
		if n != 0 or connections == None:
			connections = get_connections(field)
		vanished = []
                for c in connections:
                    if len(c) >=4:
                        score += doujikesi_bonus[len(c)]
                        vanished += c
                        
		if not vanished:
			break
                colors = set()
                for x in vanished:
                    colors.add(field[x])
                score += doujikesi_color_bonus[len(colors)]
                total_score += score * len(vanished) * 10
                vanish(field, vanished)
		
                n += 1
	return (n, total_score)

def get_heights(field):
	heights = [100]
	for x in range(1,7):
		for y in range(13, 0, -1):
			if field[x + y*8] <= 0:
				heights.append(y)
				break
	heights.append(100)
	return heights

def get_candidate_pos_internal(field, puyo):
	pos_list = []
	heights = get_heights(field)
	for x in range(1,7):
		if heights[x] > 2:
			pos_list.append((x +  heights[x]*8, x + (heights[x] - 1)*8))
		if heights[x] > 1 and heights[x+1] > 1 and x <= 5:
			pos_list.append((x + heights[x] * 8, x + 1 + heights[x+1]*8))
	return pos_list

def get_candidate_pos(field, puyo):
	pos_list = get_candidate_pos_internal(field, puyo)
	if puyo[0] != puyo[1]:
		pos_list += [(b,a) for (a,b) in pos_list]	
	return pos_list
	

def get_flatness(field):
	f = 0
        heights = get_heights(field)[1:7]
        
        d0 = heights[1] - heights[0]
        
        if d0 < 0:
            f += d0 / 1.0
        d5 = heights[5] - heights[4]

        if d5 > 0:
            f -= d5 / 1.0
        d =  max(1, (max(heights) - min(heights))) / 4.0
        f -= d 
        dekoboko = 0

        for i in range(0, 5):
            dekoboko += abs(heights[i+1] - heights[i])
        f -= dekoboko / 10.0
        return f 
        
flatness_table = [1.0, 1.0, 0.95, 0.9, 0.85, 0.7, 0.5, 0.3, 0.1, 0, 0, 0, 0, 0, 0]
connection_scores = [0.0, 0.0, 0.7, 1.0, 0.9, 0.8, 0.7, 0.6, 0.5] + [0.5]*10
def eval(field, v=False):
	#print_field(field)
	cs = get_connections(field)
	#print cs

	score = 0
	if not cs:
                return 0
        
        flatness = get_flatness(field)
	scores = []
        hakkaten = defaultdict(set) 
        seen_hakka = set()
	for c in cs:
		for x in c:
                        for dx in (-1,1,-8):
                            xx = x+dx
                            if field[xx] == 0:
                                hakkaten[(xx,field[x])].update(c)
        deduped_hakkaten = {}
        seen_hakka = set()
        for h in hakkaten:
                c = tuple(hakkaten[h])
                if c not in seen_hakka:
                       deduped_hakkaten[h] = c 
                       seen_hakka.add(c)
                    
        for h in deduped_hakkaten:
                c = hakkaten[h]

        	field_ = field[:]
		num_rensa_, score_ = fire(field_, fire_from = c)
		gomi = [True for x in xys if field_[x] != 0]

                score_ = math.log(max(score_,1))
                penalized_rensa = num_rensa_
                if len(c) == 1:
                        penalized_rensa *= 0.7
                elif(len(c)) == 2:
                        penalized_rensa *= 0.9
                
		scores.append((num_rensa_, penalized_rensa, score_, len(gomi)))

	#avg_connections = sum([connection_scores[len(x)] for x in cs]) / len(cs)
	avg_connections = sum([connection_scores[len(deduped_hakkaten[h])] for h in deduped_hakkaten]) / len(deduped_hakkaten)

        if v:
                print deduped_hakkaten 
        #print "hakkaten: ", hakkaten
	sorted_rensas = sorted(scores, key = lambda x:x[1], reverse=True)
	topn=4
        if len(sorted_rensas) == 1:
	        topn_rensa = 0
        else:
                topn_rensa = float(sum([x[1] for x in sorted_rensas[1:topn]]))/ len(sorted_rensas[1:topn])
                
	score = avg_connections*2.0 + flatness * 2.0  + topn_rensa / 2.0 + sorted_rensas[0][1]

	if v:
		print "scores: ", sorted_rensas
		print "max_rensa: ", sorted_rensas[0][0]
		print "max_score: ", sorted_rensas[0][2]
		print "gomi: ", sorted_rensas[0][3] 
		print "flatness: ", flatness
		print "avg_connections: ", avg_connections
		print "topn_rensa: ", topn_rensa
		print "total score: ", score
	#num_rensa += score
		
	
	return score 


def put(field, pos, puyo):
	field[pos[0]] = puyo[0]
	field[pos[1]] = puyo[1]
	return field

def take(field, pos, puyo):
	field[pos[0]] = 0
	field[pos[1]] = 0
	return field

def ai(field, puyos, v=0):
	max_score = -100000
	max_pos = None
	# shuffle?
	for pos1 in get_candidate_pos(field, puyos[0]):
		field1 = put(field[:], pos1, puyos[0])
		(num_rensa, score) = fire(field1)
		if num_rensa > 0:
			#for now...
			continue
		
                if v > 1:
		    print "score: %.2f rensa: %d " % (score,num_rensa)
	            print_field(field1, pos1)
		score1 = eval(field1, v=v>0)
		for pos2 in get_candidate_pos(field1, puyos[1]): 
			field2 = put(field1[:], pos2, puyos[1])
			(num_rensa, score) = fire(field2)
			if num_rensa > 0:
				#for now...
				continue
			score = eval(field2)
			if v > 1:
                            print "  score: %.2f rensa: %d" % (score, num_rensa)
			#print_field(field2, pos2)
			if num_rensa > 0:
				continue
		#	print "%.2f" % score,
                        score += score1 * 0.0001
			if score > max_score:
				max_score = score
				max_pos = (pos1, pos2)
		#print
	#print "max_score: %f" % max_score
	return max_pos[0] 


field = [-1]*8
field += [-1,0,0,0,0,0,0,-1]*13
field += [-1]*8

data = ""
V = 0
for a in sys.argv[1:]:
	try:
	    V = int(a)
	except:
            pass

if len(sys.argv) > 1:
	url = sys.argv[1]
	if url.startswith("-1"):
		for line in url.split("\n"):
			if line:
				field.append([])
				for i in range(len(line)/2):
					field[-1].append(int(line[i*2: i*2+2]))
	elif url.startswith("http"):
		if url.startswith("http://bit.ly"):
			url = urllib.urlopen(url).geturl()
		import re
		m = re.match('.*rensim/\?\?(\d+)', url)
		data =  m.group(1)
		x,y = (6,13)
		mapping = {'0':0, '4':1, '5':2, '6':3, '7':4, '8':5 }
		for d in reversed(data):
			field[x+y*8] = mapping[d] 
			x -= 1
			if x == 0:
				x = 6
				y -= 1


print_field(field)
num_rensa, score = fire(field)
eval(field, v=1)

if num_rensa > 0:
	print "%d rensa %d points" %(num_rensa, score)
	print_field(field)
        if V > 0:
        	raw_input("...")

#raw_input(".....")

random.seed(15)
puyos = []

for i in range(35):
	puyos.append((random.randint(1,4), random.randint(1,4)))

for i in range(len(puyos)-3):
	print i
	pos = ai(field, puyos[i:i+2], v=V)
	put(field, pos, puyos[i])
	print_field(field, highlight_pos = pos)
	eval(field, v=True)
	num_rensa, score = fire(field)
        print get_url(field)
	if num_rensa > 0:
		print "%d rensa %d points" %(num_rensa, score)
        if V>0:
                raw_input("...")
 
print get_url(field)
