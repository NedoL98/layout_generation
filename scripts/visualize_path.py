import argparse
import pygame as pg
import re

SCREEN_SIZE = [1440, 810]

class Assignment:
    def __init__(self, start, finish):
        self.start = start
        self.finish = finish

class Assignments:
    def __init__(self):
        self.starts = []
        self.finishes = []

    def append(self, assignment):
        self.starts.append(assignment.start)
        self.finishes.append(assignment.finish)

    def get_list(self):
        result = []
        assert(len(self.starts) == len(self.finishes))
        for i in range(len(self.starts)):
            result.append(self.starts[i])
            result.append(self.finishes[i])
        return result

    def __repr__(self):
        for start, finish in zip(self.starts, self.finishes):
            print('{{{}, {}}}'.format(start, finish))


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('queries', type=str, help='path to file with queries')
    parser.add_argument('mapf_output', type=str, help='path to file with mapf output')
    return parser.parse_args()

def split_pair_to_list(pair):
    return list(map(int, pair.split(', ')))

def parse_queries_file(queries_file_path):
    lines = open(queries_file_path, "r").readlines()
    w, h = [int(token) for token in lines[0].split(',')]
    induct_points = set()
    eject_points = set()
    for line in lines:
        if "Induct" in line:
            tokens = line.split(',')
            induct_points.add((int(tokens[3]), int(tokens[4])))
        elif "Eject" in line:
            tokens = line.split(',')
            eject_points.add((int(tokens[3]), int(tokens[4])))
    return w, h, induct_points, eject_points


def parse_path(line):
    positions_raw = line.split(':')[1]
    re_result = re.findall(r"{(\d*, \d*)}", positions_raw)
    return [split_pair_to_list(elem) for elem in re_result]

def parse_checkpoints(line):
    checkpoints_raw = line.split(':')[1]
    re_result = re.findall(r"{(\d*, \d*)}", checkpoints_raw)
    checkpoints = Assignments()
    for i in range(0, len(re_result), 2):
        start = split_pair_to_list(re_result[i])
        finish = split_pair_to_list(re_result[i + 1])
        checkpoints.append(Assignment(start, finish))
    return checkpoints

def parse_output_file(mapf_output_file_path):
    paths = []
    agents_checkpoints = []
    agent_locations_to_visit = []
    for line in open(mapf_output_file_path, "r").readlines():
        if line.startswith("Path for agent"):
            paths.append(parse_path(line))
        elif line.startswith("All assignments for agent"):
            agents_checkpoints.append(parse_checkpoints(line))
        elif line.startswith("All locations to visit for agent"):
            agent_locations_to_visit.append(parse_checkpoints(line))
    max_path_len = max([len(path) for path in paths])
    return paths, agents_checkpoints, agent_locations_to_visit

def generate_square_polygon(w_idx, h_idx, scale = 10, margin = 2):
    return [
        [w_idx * (scale + margin), h_idx * (scale + margin)],
        [(w_idx + 1) * (scale + margin) - margin, h_idx * (scale + margin)],
        [(w_idx + 1) * (scale + margin) - margin, (h_idx + 1) * (scale + margin) - margin],
        [(w_idx) * (scale + margin), (h_idx + 1) * (scale + margin) - margin]]

def draw_empty_board(screen, width, height, scale, margin, agent_checkpoints_starts, agent_checkpoints_finishes):
    total_width = width * (scale + margin)
    total_height = height * (scale + margin)
    pg.draw.polygon(screen, [0, 0, 0], [
        [0, 0],
        [total_width, 0],
        [total_width, total_height],
        [0, total_height]
    ])

    for w in range(width):
        for h in range(height):
            color = [255, 255, 200]
            assignment = (w, h)
            if assignment in agent_checkpoints_starts:
                color = [0, 255, 0]
            elif assignment in agent_checkpoints_finishes:
                color = [0, 0, 255]
            pg.draw.polygon(screen, color, generate_square_polygon(w, h, scale, margin))

def draw_positions(paths, tick, screen, width, height, agent_checkpoints_starts, agent_checkpoints_finishes):
    scale = min(SCREEN_SIZE[0] / width, (SCREEN_SIZE[1] - (len(paths) + 1) * 25) / height)
    margin = 2
    scale -= margin
    draw_empty_board(screen, width, height, scale, margin, agent_checkpoints_starts, agent_checkpoints_finishes)
    for agent_idx, path in enumerate(paths):
        pos = None
        if tick >= len(path):
            continue
        else:
            pos = path[tick]
        polygon_points = generate_square_polygon(int(pos[0]), int(pos[1]), scale, margin)
        pg.draw.polygon(screen, [0, 0, 0], polygon_points)
        screen.blit(pg.font.SysFont('Arial', int(scale)).render(str(agent_idx), True, (0, 255, 0)), polygon_points[0])
    # todo : make this work if board fits vertically
    return (margin + scale) * height

class Agent:
    def __init__(self, checkpoints, path):
        self.checkpoints = checkpoints.get_list()
        self.path = path
        self.passed_checkpoints = 0
        self.path_idx = 0
        assert len(self.checkpoints) > 0
        assert len(self.path) > 0
        if self.path[self.path_idx] == self.checkpoints[self.path_idx]:
            self.passed_checkpoints += 1

    def move_forward(self):
        self.path_idx = min(len(self.path) - 1, self.path_idx + 1)
        if self.passed_checkpoints < len(self.checkpoints) \
            and self.path[self.path_idx] == self.checkpoints[self.passed_checkpoints]:
            self.passed_checkpoints += 1

    def move_backwards(self):
        if self.passed_checkpoints > 0 \
            and self.path[self.path_idx] == self.checkpoints[self.passed_checkpoints - 1] \
            and (self.path_idx == 0 or self.path[self.path_idx - 1] != self.path[self.path_idx]):
            self.passed_checkpoints -= 1
        self.path_idx = max(0, self.path_idx - 1)

class Agents:
    def __init__(self, agents):
        self.agents = agents
        self.ts = 0

    def move_forward(self):
        self.ts += 1
        for agent in self.agents:
            if self.ts < len(agent.path):
                agent.move_forward()

    def move_backwards(self):
        self.ts -= 1
        for agent in self.agents:
            if self.ts + 1 < len(agent.path):
                agent.move_backwards()

def visualize(width, height, paths, agents_checkpoints, agents_location_to_visit,
        induct_points, eject_points):
    pg.init()
    screen = pg.display.set_mode(SCREEN_SIZE)
    screen.fill([255, 255, 255])

    clock = pg.time.Clock()
    done = False
    cur_idx = 0

    hold_left = False
    hold_right = False

    default_speed = 30
    speed_mult = 1.0

    agents = Agents([Agent(location_to_visit, path) for location_to_visit, path in zip(agents_location_to_visit, paths)])
    agent_checkpoints_starts = set()
    agent_checkpoints_finishes = set()
    for agent_checkpoints in agents_checkpoints:
        for start in agent_checkpoints.starts:
            agent_checkpoints_starts.add((start[0], start[1]))
        for finish in agent_checkpoints.finishes:
            agent_checkpoints_finishes.add((finish[0], finish[1]))

    while not done:
        screen.fill([255, 255, 255])
        board_lower_pos = draw_positions(paths, cur_idx, screen, width, height, agent_checkpoints_starts, agent_checkpoints_finishes)
        text_size = 25
        for agent_idx, agent in enumerate(agents.agents):
            text = "Agent {} passed {} checkpoints".format(str(agent_idx), agent.passed_checkpoints)
            font = pg.font.SysFont('Arial', text_size).render(text, True, (0, 0, 0))
            pos = (0, board_lower_pos + text_size * agent_idx)
            screen.blit(font, pos)
            if agent.passed_checkpoints == len(agent.checkpoints):
                font_done = pg.font.SysFont('Arial', text_size).render(" Done", True, (0, 255, 0))
                pos_done = font.get_size()
                screen.blit(font_done, (pos_done[0], board_lower_pos + text_size * agent_idx))

        text = "Timestamp: {}".format(str(cur_idx))
        font = pg.font.SysFont('Arial', text_size).render(text, True, (0, 0, 0))
        pos = (0, board_lower_pos + text_size * len(agents.agents))
        screen.blit(font, pos)

        pg.display.update()
        for e in pg.event.get():
            if e.type == pg.KEYDOWN:
                if e.key == pg.K_ESCAPE:
                    done = 1
                    break
                elif e.key == pg.K_LEFT:
                    hold_left = True
                elif e.key == pg.K_RIGHT:
                    hold_right = True
                elif e.key == pg.K_DOWN:
                    speed_mult /= 2
                elif e.key == pg.K_UP:
                    speed_mult *= 2
            elif e.type == pg.KEYUP:
                if e.key == pg.K_LEFT:
                    hold_left = False
                elif e.key == pg.K_RIGHT:
                    hold_right = False

        if hold_right:
            cur_idx += 1
            agents.move_forward()
        elif hold_left:
            cur_idx = max(cur_idx - 1, 0)
            agents.move_backwards()

        clock.tick(default_speed * speed_mult)

if __name__ == "__main__":
    args = parse_arguments()
    width, height, induct_points, eject_points = parse_queries_file(args.queries)
    paths, agents_checkpoints, agents_location_to_visit = parse_output_file(args.mapf_output)
    visualize(width, height, paths, agents_checkpoints, agents_location_to_visit, induct_points, eject_points)
